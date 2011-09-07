PKG_CHECK_MODULES(GOOGLEPINYIN "googlepinyin >= 1.0" REQUIRED )

set(FCITX_GOOGLEPINYIN_SRCS
    eim.cpp
    config.cpp
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
include_directories(${GOOGLEPINYIN_INCLUDE_DIRS})
include_directories(${LIBINTL_INCLUDE_DIR})
include_directories(${LIBICONV_INCLUDE_DIR})

link_directories(${GOOGLEPINYIN_LIBRARY_DIRS})
link_directories(${FCITX4_FCITX_LIBRARY_DIRS})
link_directories(${FCITX4_FCITX_CONFIG_LIBRARY_DIRS})
link_directories(${FCITX4_FCITX_UTILS_LIBRARY_DIRS})

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

fcitx_add_addon(fcitx-googlepinyin ${FCITX_GOOGLEPINYIN_SRCS})
target_link_libraries(fcitx-googlepinyin
                      ${GOOGLEPINYIN_LIBRARIES}
                      ${FCITX4_FCITX_LIBRARIES}
                      ${FCITX4_FCITX_CONFIG_LIBRARIES}
                      ${FCITX4_FCITX_UTILS_LIBRARIES}
                      ${LIBICONV_LIBRARIES}
                      ${LIBINTL_LIBRARIES}
                      )
