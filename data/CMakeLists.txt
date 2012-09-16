fcitx_add_addon_conf_file(fcitx-googlepinyin.conf)
fcitx_add_inputmethod_conf_file(googlepinyin.conf)

install(FILES googlepinyin.png RENAME fcitx-googlepinyin.png DESTINATION share/icons/hicolor/48x48/apps)
install(FILES googlepinyin-16.png RENAME fcitx-googlepinyin.png DESTINATION share/icons/hicolor/16x16/apps)
install(FILES googlepinyin.png DESTINATION share/fcitx/imicon RENAME googlepinyin.png)
install(FILES googlepinyin-16.png DESTINATION share/fcitx/skin/default RENAME googlepinyin.png)
install(FILES googlepinyin-16.png DESTINATION share/fcitx/skin/classic RENAME googlepinyin.png)
