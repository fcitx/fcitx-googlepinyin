/***************************************************************************
 *   Copyright (C) 2010~2010 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>
#include <fcitx-utils/log.h>
#include <fcitx/keys.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx/instance.h>
#include <fcitx-utils/utils.h>

#define _(x) gettext(x)

#include "eim.h"
#include "pinyinime.h"
#include "dictdef.h"

#ifdef __cplusplus
extern "C" {
#endif
FCITX_EXPORT_API
FcitxIMClass ime = {
    FcitxGooglePinyinCreate,
    FcitxGooglePinyinDestroy
};
#ifdef __cplusplus
}
#endif

static void FcitxGooglePinyinUpdateCand(FcitxGooglePinyin* googlepinyin);
static boolean DecodeIsDone(FcitxGooglePinyin* googlepinyin);
static void GetCCandString(FcitxGooglePinyin* googlepinyin, int index);
static void LoadGooglePinyinConfig(FcitxGooglePinyinConfig* fs, boolean reload);
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
    FcitxInstance* instance = googlepinyin->owner;
    FcitxInputState* input = &instance->input;
    int maxCand = ConfigGetMaxCandWord(&instance->config);
    size_t len;
    size_t buflen = strlen(googlepinyin->buf);
    ime_pinyin::im_get_sps_str(&len);
    if (len >= buflen)
    {
        input->iCandWordCount = ime_pinyin::im_search(googlepinyin->buf, buflen);
        input->iCandPageCount = (input->iCandWordCount +  maxCand - 1)/ maxCand - 1 ;
    }
    else {
        while (len < buflen)
        {
            input->iCandWordCount = ime_pinyin::im_search(googlepinyin->buf, len);
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
        input->iCandWordCount = ime_pinyin::im_search(googlepinyin->buf, len);
    }
    input->iCandPageCount = (input->iCandWordCount +  maxCand - 1)/ maxCand - 1 ;
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
    FcitxInputState* input = &instance->input;
    int maxCand = ConfigGetMaxCandWord(&instance->config);
    if (IsHotKeySimple(sym, state))
    {
        if (IsHotKeyLAZ(sym, state) || sym == '\'')
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
                return FcitxGooglePinyinGetCandWords(googlepinyin, SM_FIRST);
            }
            else
                return IRV_DO_NOTHING;
        }
        else if (IsHotKeyDigit(sym, state) || IsHotKey(sym, state, FCITX_SPACE))
        {
            size_t len = strlen(googlepinyin->buf);
            if (len == 0)
                return IRV_TO_PROCESS;
            
            
            int iKey = 0;
            if (IsHotKey(sym, state, FCITX_SPACE))
                iKey = 0;
            else
            {
                iKey = sym - '0';
                if (iKey == 0)
                    iKey = 10;
                iKey--;
            }
            if (iKey + input->iCurrentCandPage * maxCand >= input->iCandWordCount)
            {
                return IRV_DO_NOTHING;
            }
            else
            {
                input->iCandWordCount = ime_pinyin::im_choose(iKey + input->iCurrentCandPage * maxCand);
                input->iCandPageCount = (input->iCandWordCount +  maxCand - 1)/ maxCand -1 ;
                return FcitxGooglePinyinGetCandWords(googlepinyin, SM_FIRST);
            }
        }

    }
    if (IsHotKey(sym, state, FCITX_BACKSPACE) || IsHotKey(sym, state, FCITX_DELETE))
    {
        if (strlen(googlepinyin->buf) > 0)
        {
            if (ime_pinyin::im_get_fixed_len() != 0 && IsHotKey(sym, state, FCITX_BACKSPACE))
            {
                input->iCandWordCount = ime_pinyin::im_cancel_last_choice();
                input->iCandPageCount = (input->iCandWordCount +  maxCand - 1)/ maxCand -1 ;
            }
            else
            {
                if (IsHotKey(sym, state, FCITX_BACKSPACE))
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
            return FcitxGooglePinyinGetCandWords(googlepinyin, SM_FIRST);
        }
        else
            return IRV_TO_PROCESS;
    }
    else
    {
        
        if (strlen(googlepinyin->buf) > 0)
        {
            if (IsHotKey(sym, state, FCITX_LEFT))
            {
                const ime_pinyin::uint16* start = 0;
                ime_pinyin::im_get_spl_start_pos(start);
                size_t fixed_len = ime_pinyin::im_get_fixed_len();
                if (googlepinyin->CursorPos > 0)
                {
                    if ( googlepinyin->CursorPos == start[fixed_len])
                    {
                        input->iCandWordCount = ime_pinyin::im_cancel_last_choice();
                        input->iCandPageCount = (input->iCandWordCount +  maxCand - 1)/ maxCand - 1 ;
                        return FcitxGooglePinyinGetCandWords(googlepinyin, SM_FIRST);
                    }
                    else
                    {
                        googlepinyin->CursorPos -- ;
                        FcitxGooglePinyinUpdateCand(googlepinyin);
                        return IRV_DISPLAY_CANDWORDS;
                    }
                }

                return IRV_DO_NOTHING;
            }
            else if (IsHotKey(sym, state, FCITX_RIGHT))
            {
                size_t len = strlen(googlepinyin->buf);
                if (googlepinyin->CursorPos < (int) len)
                {
                    googlepinyin->CursorPos ++ ;
                    FcitxGooglePinyinUpdateCand(googlepinyin);
                    return IRV_DISPLAY_CANDWORDS;
                }
                return IRV_DO_NOTHING;
            }
            else if (IsHotKey(sym, state, FCITX_HOME))
            {
                const ime_pinyin::uint16* start = 0;
                ime_pinyin::im_get_spl_start_pos(start);
                size_t fixed_len = ime_pinyin::im_get_fixed_len();
                if ( googlepinyin->CursorPos != start[fixed_len])
                {
                    googlepinyin->CursorPos = start[fixed_len];
                    FcitxGooglePinyinUpdateCand(googlepinyin);
                    return IRV_DISPLAY_CANDWORDS;
                }
                return IRV_DO_NOTHING;
            }
            else if (IsHotKey(sym, state, FCITX_END))
            {
                size_t len = strlen(googlepinyin->buf);
                if (googlepinyin->CursorPos != (int) len)
                {
                    googlepinyin->CursorPos = len ;
                    FcitxGooglePinyinUpdateCand(googlepinyin);
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
    FcitxInputState* input = &instance->input;
    int maxCand = ConfigGetMaxCandWord(&instance->config);
    int startCand = maxCand * input->iCurrentCandPage;
    int endCand = maxCand * input->iCurrentCandPage + maxCand;
    if (endCand > input->iCandWordCount)
        endCand = input->iCandWordCount;

    size_t len = 0;
    FcitxLog(DEBUG, "len: %lu", len);

    SetMessageCount(GetMessageUp(instance), 0);
    if (googlepinyin->buf[0] != '\0')
    {
        const ime_pinyin::uint16* start = 0;
        char *pp = googlepinyin->ubuf;
        size_t start_pos_len = ime_pinyin::im_get_spl_start_pos(start);
        size_t fixed_len = ime_pinyin::im_get_fixed_len() * sizeof(ime_pinyin::char16);
        size_t bufsize = UTF8_BUF_LEN;
        ime_pinyin::char16* p = ime_pinyin::im_get_candidate(0, googlepinyin->retbuf, RET_BUF_LEN);
        iconv(googlepinyin->conv, (char**) &p, &fixed_len, &pp, &bufsize);
        googlepinyin->ubuf[UTF8_BUF_LEN - bufsize] = 0;

        AddMessageAtLast(GetMessageUp(instance), MSG_INPUT, "%s", googlepinyin->ubuf);
        int remainPos = googlepinyin->CursorPos - start[ime_pinyin::im_get_fixed_len()];
        if (remainPos < 0)
            googlepinyin->CursorPos = start[ime_pinyin::im_get_fixed_len()];
        input->iCursorPos = strlen(googlepinyin->ubuf);
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
                    input->iCursorPos += remainPos;
                else
                    input->iCursorPos += start[i + 1] - start[i];
            }
            remainPos -= start[i + 1] - start[i];

            AddMessageAtLast(GetMessageUp(instance), MSG_CODE, pybuf );
            if (i != start_pos_len - 1)
            {
                MessageConcatLast(GetMessageUp(instance), " ");
                if (remainPos >= 0)
                    input->iCursorPos += 1;
            }
        }
        if (strlen(googlepinyin->buf) > len)
        {
            MessageConcatLast(GetMessageUp(instance), " ");
            AddMessageAtLast(GetMessageUp(instance), MSG_CODE, googlepinyin->buf + start[start_pos_len] );

            if (remainPos >= 0)
            {
                input->iCursorPos += 1;
                if (remainPos > (int) strlen(googlepinyin->buf + start[start_pos_len]))
                    remainPos = strlen(googlepinyin->buf + start[start_pos_len]);
                input->iCursorPos += remainPos;
            }
        }
    }
    strcpy(input->strCodeInput, googlepinyin->buf);
    input->iCodeInputCount = strlen(googlepinyin->buf);
    instance->bShowCursor = true;
    SetMessageCount(GetMessageDown(instance), 0);

    int index = 0;
    for (int i = startCand ;i < endCand ; i ++, index ++)
    {
        GetCCandString(googlepinyin, i);

        char str[3] = { '\0', '\0', '\0' };
        if ( ConfigGetPointAfterNumber(&instance->config)) {
            str[1] = '.';
            str[2] = '\0';
        } else
            str[1] = '\0';

        if (index == 9)
            str[0] = '0';
        else
            str[0] = index + 1 + '0';
        AddMessageAtLast(GetMessageDown(instance), MSG_INDEX, "%s", str);

        MSG_TYPE iType = MSG_OTHER;

        AddMessageAtLast(GetMessageDown(instance), iType, "%s", googlepinyin->ubuf);

        if (i != (input->iCandWordCount - 1)) {
            MessageConcatLast(GetMessageDown(instance), " ");
        }
    }

}

boolean FcitxGooglePinyinInit(void* arg)
{
    return true;
}


/**
 * @brief function DoInput has done everything for us.
 *
 * @param searchMode
 * @return INPUT_RETURN_VALUE
 **/
__EXPORT_API
INPUT_RETURN_VALUE FcitxGooglePinyinGetCandWords(void* arg, SEARCH_MODE searchMode)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) arg;
    FcitxInstance* instance = googlepinyin->owner;
    FcitxInputState* input = &instance->input;
    switch (searchMode)
    {
    case SM_FIRST:
        input->iCurrentCandPage = 0;
        break;
    case SM_NEXT:
        if (!input->iCandPageCount)
            return IRV_TO_PROCESS;
        if (input->iCurrentCandPage == input->iCandPageCount)
            return IRV_DO_NOTHING;

        input->iCurrentCandPage++;
        break;
    case SM_PREV:
        if (!input->iCandPageCount)
            return IRV_TO_PROCESS;
        if (!input->iCurrentCandPage)
            return IRV_DO_NOTHING;

        input->iCurrentCandPage--;
        break;
    }
    if (DecodeIsDone(googlepinyin))
    {
        GetCCandString(googlepinyin, 0);
        size_t len;
        ime_pinyin::im_get_sps_str(&len);
        strcpy(GetOutputString(input), googlepinyin->ubuf);
        strcat(GetOutputString(input), googlepinyin->buf + len);
        return IRV_GET_CANDWORDS;
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
char *FcitxGooglePinyinGetCandWord (void* arg, int iIndex)
{
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) arg;
    FcitxInstance* instance = googlepinyin->owner;
    FcitxInputState* input = &instance->input;
    int maxCand = ConfigGetMaxCandWord(&instance->config);
    if (iIndex + input->iCurrentCandPage * maxCand >= input->iCandWordCount)
    {
        return NULL;
    }
    else
    {
        ime_pinyin::im_choose(iIndex + input->iCurrentCandPage * maxCand);
        if (DecodeIsDone(googlepinyin))
        {
            GetCCandString(googlepinyin, 0);
            size_t len;
            ime_pinyin::im_get_sps_str(&len);
            strcpy(GetOutputString(input), googlepinyin->ubuf);
            strcat(GetOutputString(input), googlepinyin->buf + len);
            return GetOutputString(input);
        }
        else
            FcitxGooglePinyinGetCandWords(googlepinyin, SM_FIRST);
    }
    return NULL;
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
    FcitxGooglePinyin* googlepinyin = (FcitxGooglePinyin*) fcitx_malloc0(sizeof(FcitxGooglePinyin));
    bindtextdomain("fcitx-googlepinyin", LOCALEDIR);
    LoadGooglePinyinConfig(&googlepinyin->config, false);
    char* userDict;
    googlepinyin->owner = instance;

    googlepinyin->conv = iconv_open("utf8", "utf16");
    FILE* fp = GetXDGFileUserWithPrefix("googlepinyin", "userdict_pinyin.dat", "a", &userDict);
    if (fp)
        fclose(fp);

    ime_pinyin::im_open_decoder(PKGDATADIR "/googlepinyin/dict_pinyin.dat", userDict);

    FcitxRegisterIM(instance,
                    googlepinyin,
                    _("GooglePinyin"),
                    "googlepinyin",
                    FcitxGooglePinyinInit,
                    FcitxGooglePinyinReset,
                    FcitxGooglePinyinDoInput,
                    FcitxGooglePinyinGetCandWords,
                    FcitxGooglePinyinGetCandWord,
                    NULL,
                    SaveFcitxGooglePinyin,
                    ReloadConfigFcitxGooglePinyin,
                    NULL,
                    googlepinyin->config.iPriority
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
    LoadGooglePinyinConfig(&googlepinyin->config, false);
}

/**
 * @brief Load the config file for fcitx-googlepinyin
 *
 * @param Bool is reload or not
 **/
void LoadGooglePinyinConfig(FcitxGooglePinyinConfig* fs, boolean reload)
{
    ConfigFileDesc *configDesc = GetGooglePinyinConfigDesc();

    FILE *fp = GetXDGFileUserWithPrefix("conf", "fcitx-googlepinyin.config", "rt", NULL);

    if (!fp)
    {
        if (!reload && errno == ENOENT)
        {
            SaveGooglePinyinConfig(fs);
            LoadGooglePinyinConfig(fs, true);
        }
        return;
    }
    ConfigFile *cfile = ParseConfigFileFp(fp, configDesc);

    if (cfile)
    {
        FcitxGooglePinyinConfigConfigBind(fs, cfile, configDesc);
        ConfigBindSync(&fs->gconfig);
    }
    else
    {
        fs->iPriority = 1;
    }
}

/**
 * @brief Save the config
 *
 * @return void
 **/
void SaveGooglePinyinConfig(FcitxGooglePinyinConfig* fs)
{
    ConfigFileDesc *configDesc = GetGooglePinyinConfigDesc();
    FILE *fp = GetXDGFileUserWithPrefix("conf", "fcitx-googlepinyin.config", "wt", NULL);
    SaveConfigFileFp(fp, &fs->gconfig, configDesc);
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
    iconv(googlepinyin->conv, (char**) &p, &strl, &pp, &bufsize);
    googlepinyin->ubuf[UTF8_BUF_LEN - bufsize] = 0;
}

void SaveFcitxGooglePinyin(void* arg)
{
    ime_pinyin::im_flush_cache();
}
