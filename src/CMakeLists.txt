PKG_CHECK_MODULES(GOOGLEPINYIN "googlepinyin >= 1.0" REQUIRED )

set(FCITX_GOOGLEPINYIN_SRCS
    eim.cpp
    config.cpp
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${GOOGLEPINYIN_INCLUDE_DIRS})

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

fcitx_add_addon(fcitx-googlepinyin ${FCITX_GOOGLEPINYIN_SRCS})
target_link_libraries(fcitx-googlepinyin ${GOOGLEPINYIN_LIBRARIES} ${FCITX4_FCITX_LIBRARIES} ${FCITX4_FCITX_CONFIG_LIBRARIES})
