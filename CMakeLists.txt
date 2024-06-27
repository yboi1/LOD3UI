cmake_minimum_required(VERSION 3.17)
project (LOD3UI)

set(qtpath $ENV{QtPath})

if(NOT qtpath)
    message(STATUS "error!")
endif()

    include_directories($ENV{QtPath}/include)
    include_directories($ENV{QtPath}/include/QtGui)
    include_directories($ENV{QtPath}/include/QtCore)
    include_directories($ENV{QtPath}/include/QtWidgets)

    file(GLOB src *.c*)
    file(GLOB include *.h*)

    link_directories($ENV{QtPath}/lib)
    add_executable(LOD3UI ${src} ${include})

    target_link_libraries(LOD3UI Qt5Core.lib)
    target_link_libraries(LOD3UI Qt5Gui.lib)
    target_link_libraries(LOD3UI Qt5Widgets.lib)




