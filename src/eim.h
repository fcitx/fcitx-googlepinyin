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

#define RET_BUF_LEN 256
#define UTF8_BUF_LEN 4096
#define MAX_GOOGLEPINYIN_INPUT MAX_USER_INPUT

struct FcitxInstance;

typedef struct FcitxGooglePinyinConfig {
    GenericConfig gconfig;
    int iPriority;
} FcitxGooglePinyinConfig;

typedef struct FcitxGooglePinyin
{
    FcitxGooglePinyinConfig config;
    FcitxInstance* owner;
    iconv_t conv;
    char buf[MAX_GOOGLEPINYIN_INPUT + 1];
    char ubuf[UTF8_BUF_LEN + 1];
    ime_pinyin::char16 retbuf[RET_BUF_LEN];
    ime_pinyin::char16 retbuf2[RET_BUF_LEN];
    int CursorPos;
} FcitxGooglePinyin;

__EXPORT_API void* FcitxGooglePinyinCreate(FcitxInstance* instance);
__EXPORT_API void FcitxGooglePinyinDestroy(void* arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxGooglePinyinDoInput(void* arg, FcitxKeySym sym, unsigned int state);
__EXPORT_API INPUT_RETURN_VALUE FcitxGooglePinyinGetCandWords (void *arg, SEARCH_MODE mode);
__EXPORT_API char *FcitxGooglePinyinGetCandWord (void *arg, int iIndex);
__EXPORT_API boolean FcitxGooglePinyinInit(void*);
__EXPORT_API void ReloadConfigFcitxGooglePinyin(void*);
__EXPORT_API void SaveFcitxGooglePinyin(void* arg);;


CONFIG_BINDING_DECLARE(FcitxGooglePinyinConfig);

#endif