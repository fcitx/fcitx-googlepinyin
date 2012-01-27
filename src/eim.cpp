/***************************************************************************
 *   Copyright (C) 2010~2011 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <libintl.h>
#include <iconv.h>

#include <fcitx/ime.h>
#include <fcitx-utils/log.h>
#include <fcitx/keys.h>
#include <fcitx/instance.h>
#include <fcitx/context.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>
#include <fcitx-utils/utils.h>

#define _(x) dgettext("fcitx-googlepinyin", (x))

#include "eim.h"
#include "pinyinime.h"
#include "dictdef.h"
#include <fcitx/candidate.h>

#ifdef LIBICONV_SECOND_ARGUMENT_IS_CONST
typedef const char** IconvStr;
#else
typedef char** IconvStr;
#endif

#ifdef __cplusplus
extern "C" {
#endif
    FCITX_EXPORT_API
    FcitxIMClass ime = {
        FcitxGooglePinyinCreate,
        FcitxGooglePinyinDestroy
    };

    FCITX_EXPORT_API
    int ABI_VERSION = FCITX_ABI_VERSION;
#ifdef __cplusplus
}
#endif

static void FcitxGooglePinyinUpdateCand(FcitxGooglePinyin* googlepinyin);
static boolean DecodeIsDone(FcitxGooglePinyin* googlepinyin);
static void GetCCandString(FcitxGooglePinyin* googlepinyin, int index);
static boolean LoadGooglePinyinConfig(FcitxGooglePinyinConfig* fs);
static void SaveGooglePinyinConfig(FcitxGooglePinyinConfig* fs);

CONFIG_DESC_DEFINE(GetGooglePinyinConfigDesc, "fcitx-googlepinyin.desc")

/**
 * @brief Reset the status.
 *
 **/
__EXPORT_API
void FcitxGooglePinyinReset (void* arg)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) arg;
    ime_pinyin::im_reset_search();
    googlepinyin->buf[0] = '\0';
    googlepinyin->CursorPos = 0;
}

/**
 * @brief googlepinyin engine is crapy, try search carefully.
 */
void TryBestSearch(FcitxGooglePinyin* googlepinyin)
{
    size_t len;
    size_t buflen = strlen(googlepinyin->buf);
    ime_pinyin::im_get_sps_str(&len);
    if (len >= buflen)
    {
        googlepinyin->candNum = ime_pinyin::im_search(googlepinyin->buf, buflen);
    }
    else
    {
        while (len < buflen)
        {
            googlepinyin->candNum = ime_pinyin::im_search(googlepinyin->buf, len);
            size_t new_len;
            ime_pinyin::im_get_sps_str(&new_len);
            if (new_len < len)
            {
                len = new_len;
                break;
            }
            if (new_len >= len)
                len ++;
        }
        googlepinyin->candNum = ime_pinyin::im_search(googlepinyin->buf, len);
    }
}

/**
 * @brief Process Key Input and return the status
 *
 * @param keycode keycode from XKeyEvent
 * @param state state from XKeyEvent
 * @param count count from XKeyEvent
 * @return INPUT_RETURN_VALUE
 **/
__EXPORT_API
INPUT_RETURN_VALUE FcitxGooglePinyinDoInput(void* arg, FcitxKeySym sym, unsigned int state)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) arg;
    FcitxInstance* instance = googlepinyin->owner;
    FcitxInputState* input = FcitxInstanceGetInputState(instance);
    if (FcitxHotkeyIsHotKeySimple(sym, state))
    {
        if (FcitxHotkeyIsHotKeyLAZ(sym, state) || sym == '\'')
        {
            if (strlen(googlepinyin->buf) < MAX_GOOGLEPINYIN_INPUT)
            {
                size_t len = strlen(googlepinyin->buf);
                if (googlepinyin->buf[googlepinyin->CursorPos] != 0)
                {
                    memmove(googlepinyin->buf + googlepinyin->CursorPos + 1, googlepinyin->buf + googlepinyin->CursorPos, len - googlepinyin->CursorPos);
                }
                googlepinyin->buf[len + 1] = 0;
                googlepinyin->buf[googlepinyin->CursorPos] = (char) (sym & 0xff);
                googlepinyin->CursorPos ++;
                TryBestSearch(googlepinyin);
                ime_pinyin::im_get_sps_str(&len);
                if (len == 0 && strlen(googlepinyin->buf) == 1)
                {
                    FcitxGooglePinyinReset(googlepinyin);
                    return IRV_TO_PROCESS;
                }
                return IRV_DISPLAY_CANDWORDS;
            }
            else
                return IRV_DO_NOTHING;
        }
        else if (FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE))
        {
            size_t len = strlen(googlepinyin->buf);
            if (len == 0)
                return IRV_TO_PROCESS;

            return FcitxCandidateWordChooseByIndex(FcitxInputStateGetCandidateList(input), 0);
        }

    }
    if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE) || FcitxHotkeyIsHotKey(sym, state, FCITX_DELETE))
    {
        if (strlen(googlepinyin->buf) > 0)
        {
            if (ime_pinyin::im_get_fixed_len() != 0 && FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE))
            {
                googlepinyin->candNum = ime_pinyin::im_cancel_last_choice();
            }
            else
            {
                if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE))
                {
                    if (googlepinyin->CursorPos > 0)
                        googlepinyin->CursorPos -- ;
                    else
                        return IRV_DO_NOTHING;
                }
                size_t len = strlen(googlepinyin->buf);
                if (googlepinyin->CursorPos == (int)len)
                    return IRV_DO_NOTHING;
                memmove(googlepinyin->buf + googlepinyin->CursorPos, googlepinyin->buf + googlepinyin->CursorPos + 1, len - googlepinyin->CursorPos - 1);
                googlepinyin->buf[strlen(googlepinyin->buf) - 1] = 0;
                TryBestSearch(googlepinyin);
            }
            return IRV_DISPLAY_CANDWORDS;
        }
        else
            return IRV_TO_PROCESS;
    }
    else
    {

        if (strlen(googlepinyin->buf) > 0)
        {
            if (FcitxHotkeyIsHotKey(sym, state, FCITX_LEFT))
            {
                const ime_pinyin::uint16* start = 0;
                ime_pinyin::im_get_spl_start_pos(start);
                size_t fixed_len = ime_pinyin::im_get_fixed_len();
                if (googlepinyin->CursorPos > 0)
                {
                    if ( googlepinyin->CursorPos == start[fixed_len])
                    {
                        googlepinyin->candNum = ime_pinyin::im_cancel_last_choice();
                        return IRV_DISPLAY_CANDWORDS;
                    }
                    else
                    {
                        googlepinyin->CursorPos -- ;
                        return IRV_DISPLAY_CANDWORDS;
                    }
                }

                return IRV_DO_NOTHING;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_RIGHT))
            {
                size_t len = strlen(googlepinyin->buf);
                if (googlepinyin->CursorPos < (int) len)
                {
                    googlepinyin->CursorPos ++ ;
                    return IRV_DISPLAY_CANDWORDS;
                }
                return IRV_DO_NOTHING;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_HOME))
            {
                const ime_pinyin::uint16* start = 0;
                ime_pinyin::im_get_spl_start_pos(start);
                size_t fixed_len = ime_pinyin::im_get_fixed_len();
                if ( googlepinyin->CursorPos != start[fixed_len])
                {
                    googlepinyin->CursorPos = start[fixed_len];
                    return IRV_DISPLAY_CANDWORDS;
                }
                return IRV_DO_NOTHING;
            }
            else if (FcitxHotkeyIsHotKey(sym, state, FCITX_END))
            {
                size_t len = strlen(googlepinyin->buf);
                if (googlepinyin->CursorPos != (int) len)
                {
                    googlepinyin->CursorPos = len ;
                    return IRV_DISPLAY_CANDWORDS;
                }
                return IRV_DO_NOTHING;
            }
        }
        else {
            return IRV_TO_PROCESS;
        }
    }
    return IRV_TO_PROCESS;
}

void FcitxGooglePinyinUpdateCand(FcitxGooglePinyin* googlepinyin)
{
    FcitxInstance* instance = googlepinyin->owner;
    FcitxInputState* input = FcitxInstanceGetInputState(instance);

    size_t len = 0;
    FcitxLog(DEBUG, "len: %lu", len);

    FcitxInstanceCleanInputWindowUp(instance);
    if (googlepinyin->buf[0] != '\0')
    {
        const ime_pinyin::uint16* start = 0;
        char *pp = googlepinyin->ubuf;
        size_t start_pos_len = ime_pinyin::im_get_spl_start_pos(start);
        size_t fixed_len = ime_pinyin::im_get_fixed_len() * sizeof(ime_pinyin::char16);
        size_t bufsize = UTF8_BUF_LEN;
        ime_pinyin::char16* p = ime_pinyin::im_get_candidate(0, googlepinyin->retbuf, RET_BUF_LEN);
        iconv(googlepinyin->conv, (IconvStr) &p, &fixed_len, &pp, &bufsize);
        googlepinyin->ubuf[UTF8_BUF_LEN - bufsize] = 0;

        FcitxMessagesAddMessageAtLast(FcitxInputStateGetPreedit(input), MSG_INPUT, "%s", googlepinyin->ubuf);
        int remainPos = googlepinyin->CursorPos - start[ime_pinyin::im_get_fixed_len()];
        if (remainPos < 0)
            googlepinyin->CursorPos = start[ime_pinyin::im_get_fixed_len()];
        int iCursorPos = strlen(googlepinyin->ubuf);
        for (size_t i = ime_pinyin::im_get_fixed_len();
                i < start_pos_len;
                i ++)
        {
            char pybuf[ime_pinyin::kMaxPinyinSize + 2]; /* py */
            const char* pystr = ime_pinyin::im_get_sps_str(&len);
            strncpy(pybuf, pystr + start[i], start[i + 1] - start[i]);
            pybuf[  start[i + 1] - start[i] ] = 0;
            if (remainPos >= 0)
            {
                if (remainPos < start[i + 1] - start[i])
                    iCursorPos += remainPos;
                else
                    iCursorPos += start[i + 1] - start[i];
            }
            remainPos -= start[i + 1] - start[i];

            FcitxMessagesAddMessageAtLast(FcitxInputStateGetPreedit(input), MSG_CODE, pybuf );
            if (i != start_pos_len - 1)
            {
                FcitxMessagesMessageConcatLast(FcitxInputStateGetPreedit(input), " ");
                if (remainPos >= 0)
                    iCursorPos += 1;
            }
        }
        if (strlen(googlepinyin->buf) > len)
        {
            FcitxMessagesMessageConcatLast(FcitxInputStateGetPreedit(input), " ");
            FcitxMessagesAddMessageAtLast(FcitxInputStateGetPreedit(input), MSG_CODE, googlepinyin->buf + start[start_pos_len] );

            if (remainPos >= 0)
            {
                iCursorPos += 1;
                if (remainPos > (int) strlen(googlepinyin->buf + start[start_pos_len]))
                    remainPos = strlen(googlepinyin->buf + start[start_pos_len]);
                iCursorPos += remainPos;
            }
        }
        FcitxInputStateSetCursorPos(input, iCursorPos);
        FcitxInputStateSetClientCursorPos(input, 0);
    }
    strcpy(FcitxInputStateGetRawInputBuffer(input), googlepinyin->buf);
    FcitxInputStateSetRawInputBufferSize(input, strlen(googlepinyin->buf));
    FcitxInputStateSetShowCursor(input, true);
    FcitxInstanceCleanInputWindowDown(instance);

    int index = 0;
    for (int i = 0 ;i < googlepinyin->candNum ; i ++, index ++)
    {
        GetCCandString(googlepinyin, i);
        GooglePinyinCandWord* ggCand = (GooglePinyinCandWord*) fcitx_utils_malloc0(sizeof(GooglePinyinCandWord));
        ggCand->index = i;
        FcitxCandidateWord candWord;
        candWord.callback = FcitxGooglePinyinGetCandWord;
        candWord.owner = googlepinyin;
        candWord.priv = ggCand;
        candWord.strExtra = NULL;
        candWord.strWord = strdup(googlepinyin->ubuf);
        candWord.wordType = MSG_OTHER;
        FcitxCandidateWordAppend(FcitxInputStateGetCandidateList(input), &candWord);
        
        if (i == 0)
            FcitxMessagesAddMessageAtLast(FcitxInputStateGetClientPreedit(input), MSG_INPUT, "%s", candWord.strWord);
    }
    
}

boolean FcitxGooglePinyinInit(void* arg)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) arg;
    FcitxInstanceSetContext(googlepinyin->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "us");
    return true;
}


/**
 * @brief function DoInput has done everything for us.
 *
 * @param searchMode
 * @return INPUT_RETURN_VALUE
 **/
__EXPORT_API
INPUT_RETURN_VALUE FcitxGooglePinyinGetCandWords(void* arg)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) arg;
    FcitxInstance* instance = googlepinyin->owner;
    FcitxInputState* input = FcitxInstanceGetInputState(instance);
    FcitxGlobalConfig* config = FcitxInstanceGetGlobalConfig(googlepinyin->owner);

    FcitxCandidateWordSetPageSize(FcitxInputStateGetCandidateList(input), config->iMaxCandWord);
    FcitxCandidateWordSetChoose(FcitxInputStateGetCandidateList(input), DIGIT_STR_CHOOSE);

    if (DecodeIsDone(googlepinyin))
    {
        GetCCandString(googlepinyin, 0);
        size_t len;
        ime_pinyin::im_get_sps_str(&len);
        strcpy(FcitxInputStateGetOutputString(input), googlepinyin->ubuf);
        strcat(FcitxInputStateGetOutputString(input), googlepinyin->buf + len);
        if (strlen(FcitxInputStateGetOutputString(input)) == 0)
            return IRV_CLEAN;
        else
            return IRV_COMMIT_STRING;
    }
    FcitxGooglePinyinUpdateCand(googlepinyin);
    if (googlepinyin->buf[0] == '\0')
        return IRV_CLEAN;

    return IRV_DISPLAY_CANDWORDS;
}

/**
 * @brief get the candidate word by index
 *
 * @param iIndex index of candidate word
 * @return the string of canidate word
 **/
__EXPORT_API
INPUT_RETURN_VALUE FcitxGooglePinyinGetCandWord (void* arg, FcitxCandidateWord* candWord)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) arg;
    FcitxInstance* instance = googlepinyin->owner;
    FcitxInputState* input = FcitxInstanceGetInputState(instance);
    GooglePinyinCandWord* ggCand = (GooglePinyinCandWord*) candWord->priv;

    googlepinyin->candNum = ime_pinyin::im_choose(ggCand->index);
    if (DecodeIsDone(googlepinyin))
    {
        GetCCandString(googlepinyin, 0);
        size_t len;
        ime_pinyin::im_get_sps_str(&len);
        strcpy(FcitxInputStateGetOutputString(input), googlepinyin->ubuf);
        strcat(FcitxInputStateGetOutputString(input), googlepinyin->buf + len);
        if (strlen(FcitxInputStateGetOutputString(input)) == 0)
            return IRV_CLEAN;
        else
            return IRV_COMMIT_STRING;
    }
    else
        return IRV_DISPLAY_CANDWORDS;
}

/**
 * @brief initialize the extra input method
 *
 * @param arg
 * @return successful or not
 **/
__EXPORT_API
void* FcitxGooglePinyinCreate (FcitxInstance* instance)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) fcitx_utils_malloc0(sizeof(FcitxGooglePinyin));
    bindtextdomain("fcitx-googlepinyin", LOCALEDIR);
    if (!LoadGooglePinyinConfig(&googlepinyin->config))
    {
        free(googlepinyin);
        return NULL;
    }
    char* userDict = NULL;
    googlepinyin->owner = instance;

    union {
        short s;
        unsigned char b[2];
    } endian;

    endian.s = 0x1234;
    if (endian.b[0] == 0x12)
        googlepinyin->conv = iconv_open("utf-8", "utf-16be");
    else
        googlepinyin->conv = iconv_open("utf-8", "utf-16le");
    if (googlepinyin->conv == (iconv_t)(-1))
    {
        free(googlepinyin);
        return NULL;
    }
    FILE* fp = FcitxXDGGetFileUserWithPrefix("googlepinyin", "userdict_pinyin.dat", "a", &userDict);
    if (fp)
        fclose(fp);

    bool result = ime_pinyin::im_open_decoder(LIBDIR "/googlepinyin/data/dict_pinyin.dat", userDict);

    if (userDict)
        free(userDict);

    if (!result)
    {
        free(googlepinyin);
        return NULL;
    }


    FcitxInstanceRegisterIM(instance,
                    googlepinyin,
                    "googlepinyin",
                    _("GooglePinyin"),
                    "googlepinyin",
                    FcitxGooglePinyinInit,
                    FcitxGooglePinyinReset,
                    FcitxGooglePinyinDoInput,
                    FcitxGooglePinyinGetCandWords,
                    NULL,
                    SaveFcitxGooglePinyin,
                    ReloadConfigFcitxGooglePinyin,
                    NULL,
                    googlepinyin->config.iPriority,
                    "zh_CN"
                   );
    return googlepinyin;
}

/**
 * @brief Destroy the input method while unload it.
 *
 * @return int
 **/
__EXPORT_API
void FcitxGooglePinyinDestroy (void* arg)
{
    free(arg);
}

__EXPORT_API void ReloadConfigFcitxGooglePinyin(void* arg)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) arg;
    LoadGooglePinyinConfig(&googlepinyin->config);
}

/**
 * @brief Load the config file for fcitx-googlepinyin
 *
 * @param Bool is reload or not
 **/
boolean LoadGooglePinyinConfig(FcitxGooglePinyinConfig* fs)
{
    FcitxConfigFileDesc *configDesc = GetGooglePinyinConfigDesc();
    if (configDesc == NULL)
        return false;

    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-googlepinyin.config", "rt", NULL);

    if (!fp)
    {
        if (errno == ENOENT)
            SaveGooglePinyinConfig(fs);
    }
    FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, configDesc);
    FcitxGooglePinyinConfigConfigBind(fs, cfile, configDesc);
    FcitxConfigBindSync(&fs->gconfig);

    if (fp)
        fclose(fp);

    return true;
}

/**
 * @brief Save the config
 *
 * @return void
 **/
void SaveGooglePinyinConfig(FcitxGooglePinyinConfig* fs)
{
    FcitxConfigFileDesc *configDesc = GetGooglePinyinConfigDesc();
    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-googlepinyin.config", "wt", NULL);
    FcitxConfigSaveConfigFileFp(fp, &fs->gconfig, configDesc);
    if (fp)
        fclose(fp);
}


boolean DecodeIsDone(FcitxGooglePinyin* googlepinyin)
{
    const ime_pinyin::uint16* start = 0;
    size_t len;
    ime_pinyin::im_get_spl_start_pos(start);
    ime_pinyin::im_get_sps_str(&len);
    return (len == start[ime_pinyin::im_get_fixed_len()]);
}

void GetCCandString(FcitxGooglePinyin* googlepinyin, int index)
{
    char *pp = googlepinyin->ubuf;
    size_t bufsize = UTF8_BUF_LEN;
    ime_pinyin::char16* p = ime_pinyin::im_get_candidate(index, googlepinyin->retbuf, RET_BUF_LEN);
    size_t strl = ime_pinyin::utf16_strlen(p) * sizeof(ime_pinyin::char16);
    iconv(googlepinyin->conv, (IconvStr) &p, &strl, &pp, &bufsize);
    googlepinyin->ubuf[UTF8_BUF_LEN - bufsize] = 0;
}

void SaveFcitxGooglePinyin(void* arg)
{
    ime_pinyin::im_flush_cache();
}
// kate: indent-mode cstyle; space-indent on; indent-width 0;
