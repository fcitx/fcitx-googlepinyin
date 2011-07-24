project(fcitx-googlepinyin)
cmake_minimum_required(VERSION 2.6)
FIND_PACKAGE(Gettext REQUIRED)
FIND_PACKAGE(Fcitx4 REQUIRED)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

include(FindPkgConfig)

set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(bindir ${prefix}/bin)
set(libdir ${LIB_INSTALL_DIR})
set(addondir ${libdir}/${package} )
set(pkgdatadir ${CMAKE_INSTALL_PREFIX}/share/${package})
set(localedir ${CMAKE_INSTALL_PREFIX}/share/locale)
set(includedir  ${CMAKE_INSTALL_PREFIX}/include)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined,--as-needed")
configure_file(config.h.in config.h)

set(libdir ${LIB_INSTALL_DIR})

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)