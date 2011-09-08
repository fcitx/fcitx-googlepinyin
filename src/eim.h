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

#ifndef EIM_H
#define EIM_H

#ifdef __cplusplus
#define __EXPORT_API extern "C"
#else
#define __EXPORT_API
#endif

#include <iconv.h>
#include <fcitx/ime.h>
#include <pinyinime.h>
#include <fcitx/instance.h>
#include <fcitx/candidate.h>

#define RET_BUF_LEN 256
#define UTF8_BUF_LEN 4096
#define MAX_GOOGLEPINYIN_INPUT MAX_USER_INPUT

typedef struct _FcitxGooglePinyinConfig {
    GenericConfig gconfig;
    int iPriority;
} FcitxGooglePinyinConfig;

typedef struct _GooglePinyinCandWord {
    int index;
} GooglePinyinCandWord;

typedef struct _FcitxGooglePinyin
{
    FcitxGooglePinyinConfig config;
    FcitxInstance* owner;
    iconv_t conv;
    char buf[MAX_GOOGLEPINYIN_INPUT + 1];
    char ubuf[UTF8_BUF_LEN + 1];
    ime_pinyin::char16 retbuf[RET_BUF_LEN];
    ime_pinyin::char16 retbuf2[RET_BUF_LEN];
    int CursorPos;
    int candNum;
} FcitxGooglePinyin;

__EXPORT_API void* FcitxGooglePinyinCreate(FcitxInstance* instance);
__EXPORT_API void FcitxGooglePinyinDestroy(void* arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxGooglePinyinDoInput(void* arg, FcitxKeySym sym, unsigned int state);
__EXPORT_API INPUT_RETURN_VALUE FcitxGooglePinyinGetCandWords (void *arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxGooglePinyinGetCandWord (void* arg, CandidateWord* candWord);
__EXPORT_API boolean FcitxGooglePinyinInit(void*);
__EXPORT_API void ReloadConfigFcitxGooglePinyin(void*);
__EXPORT_API void SaveFcitxGooglePinyin(void* arg);;


CONFIG_BINDING_DECLARE(FcitxGooglePinyinConfig);

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 0;
