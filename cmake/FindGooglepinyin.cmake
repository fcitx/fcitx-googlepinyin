# - Try to find the Googlepinyin libraries
# Once done this will define
#
#  GOOGLEPINYIN_FOUND - system has GOOGLEPINYIN
#  GOOGLEPINYIN_INCLUDE_DIR - the GOOGLEPINYIN include directory
#  GOOGLEPINYIN_LIBRARIES - GOOGLEPINYIN library
#
# Copyright (c) 2012 CSSlayer <wengxt@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(GOOGLEPINYIN_INCLUDE_DIR AND GOOGLEPINYIN_LIBRARIES)
    # Already in cache, be silent
    set(GOOGLEPINYIN_FIND_QUIETLY TRUE)
endif(GOOGLEPINYIN_INCLUDE_DIR AND GOOGLEPINYIN_LIBRARIES)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBGOOGLEPINYIN "googlepinyin>=1.0")

find_path(GOOGLEPINYIN_MAIN_INCLUDE_DIR
          NAMES pinyinime.h
          HINTS ${PC_LIBGOOGLEPINYIN_INCLUDEDIR})

find_library(GOOGLEPINYIN_LIBRARIES
             NAMES googlepinyin
             HINTS ${PC_LIBGOOGLEPINYIN_LIBDIR})

set(GOOGLEPINYIN_INCLUDE_DIR "${GOOGLEPINYIN_MAIN_INCLUDE_DIR}")
set(GOOGLEPINYIN_LIBDIR "${PC_LIBGOOGLEPINYIN_LIBDIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Googlepinyin  DEFAULT_MSG 
                                  GOOGLEPINYIN_LIBRARIES
                                  GOOGLEPINYIN_MAIN_INCLUDE_DIR
                                  )

mark_as_advanced(GOOGLEPINYIN_MAIN_INCLUDE_DIR GOOGLEPINYIN_INCLUDE_DIR GOOGLEPINYIN_LIBRARIES GOOGLEPINYIN_LIBDIR)
