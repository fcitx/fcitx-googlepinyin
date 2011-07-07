set(FCITX_GOOGLEPINYIN_SRCS
    eim.cpp
    config.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/libgooglepinyin/include)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${GOOGLEPINYIN_INCLUDE_DIRS})

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

add_library(fcitx-googlepinyin MODULE ${FCITX_GOOGLEPINYIN_SRCS})
set_target_properties( fcitx-googlepinyin PROPERTIES PREFIX "")
target_link_libraries(fcitx-googlepinyin googlepinyin)
install(TARGETS fcitx-googlepinyin DESTINATION ${libdir}/fcitx/)
