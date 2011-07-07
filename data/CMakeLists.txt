add_custom_command(OUTPUT desc.po
                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/getdescpo ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(desc ALL DEPENDS desc.po)

install(FILES fcitx-googlepinyin.conf DESTINATION share/fcitx/data/addon)
install(FILES fcitx-googlepinyin.desc DESTINATION share/fcitx/data/addon)
