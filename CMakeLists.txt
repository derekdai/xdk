cmake_minimum_required( VERSION 2.8 )

project( test-xlib )

find_package( PkgConfig )

pkg_check_modules( X11 REQUIRED x11 )
pkg_check_modules( GLIB REQUIRED glib-2.0 )

include_directories( ${X11_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} )

link_directories( ${X11_LIBRARY_DIRS} ${GLIB_LIBRARY_DIRS} )

add_executable( simple-window simple-window )
target_link_libraries( simple-window ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )

add_executable( simple-window-keypress-draw simple-window-keypress-draw )
target_link_libraries( simple-window-keypress-draw ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )

add_executable( simple-draw-with-color simple-draw-with-color )
target_link_libraries( simple-draw-with-color ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )
