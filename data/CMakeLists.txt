add_custom_command(OUTPUT desc.po
                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/getdescpo ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS fcitx-googlepinyin.desc
                  )

add_custom_target(desc ALL DEPENDS desc.po)

fcitx_add_addon_conf_file(fcitx-googlepinyin.conf)
fcitx_add_configdesc_file(fcitx-googlepinyin.desc)
