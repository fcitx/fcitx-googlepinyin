PKG_CHECK_MODULES(GOOGLEPINYIN "googlepinyin >= 1.0" REQUIRED )

set(FCITX_GOOGLEPINYIN_SRCS
    eim.cpp
    config.cpp
)

include_directories(
    ${PROJECT_BINARY_DIR}
    ${GOOGLEPINYIN_INCLUDE_DIRS}
    ${LIBINTL_INCLUDE_DIR}
    ${LIBICONV_INCLUDE_DIR}
    ${FCITX4_FCITX_INCLUDE_DIRS}
    ${FCITX4_FCITX_CONFIG_INCLUDE_DIRS}
    ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
)

link_directories(${GOOGLEPINYIN_LIBRARY_DIRS})

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

fcitx_add_addon(fcitx-googlepinyin ${FCITX_GOOGLEPINYIN_SRCS})
target_link_libraries(fcitx-googlepinyin
                      ${GOOGLEPINYIN_LIBRARIES}
                      ${LIBICONV_LIBRARIES}
                      ${LIBINTL_LIBRARIES}
                      )
