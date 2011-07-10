PKG_CHECK_MODULES(GOOGLEPINYIN "googlepinyin >= 1.0" )
PKG_CHECK_MODULES(FCITX_CONFIG REQUIRED "fcitx-config >= 4.1" )
PKG_CHECK_MODULES(FCITX REQUIRED "fcitx >= 4.1" )

set(FCITX_GOOGLEPINYIN_SRCS
    eim.cpp
    config.cpp
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${GOOGLEPINYIN_INCLUDE_DIRS})

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

add_library(fcitx-googlepinyin MODULE ${FCITX_GOOGLEPINYIN_SRCS})
set_target_properties( fcitx-googlepinyin PROPERTIES PREFIX "")
target_link_libraries(fcitx-googlepinyin ${GOOGLEPINYIN_LIBRARIES} ${FCITX_LIBRARIES} ${FCITX_CONFIG_LIBRARIES})
install(TARGETS fcitx-googlepinyin DESTINATION ${libdir}/fcitx/)
