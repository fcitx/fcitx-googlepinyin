fcitx_add_addon_conf_file(fcitx-googlepinyin.conf)
fcitx_add_configdesc_file(fcitx-googlepinyin.desc)

if(NOT (${FCITX4_VERSION} STREQUAL "4.1.2"))
    fcitx_add_inputmethod_conf_file(googlepinyin.conf)
endif(NOT (${FCITX4_VERSION} STREQUAL "4.1.2"))