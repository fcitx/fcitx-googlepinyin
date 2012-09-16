fcitx_add_addon_conf_file(fcitx-googlepinyin.conf)
fcitx_add_inputmethod_conf_file(googlepinyin.conf)

install(FILES googlepinyin.png RENAME fcitx-googlepinyin.png DESTINATION share/icons/hicolor/48x48/apps)
install(FILES googlepinyin.png DESTINATION share/fcitx/imicon RENAME googlepinyin.png)
