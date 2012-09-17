set( libraries ${X11_LIBRARIES} ${GLIB_LIBRARIES} )

add_executable( simple-window simple-window )
target_link_libraries( simple-window ${libraries} )

add_executable( simple-window-wm-delete-window simple-window-wm-delete-window )
target_link_libraries( simple-window-wm-delete-window ${libraries} )

add_executable( simple-window-keypress-draw simple-window-keypress-draw )
target_link_libraries( simple-window-keypress-draw ${libraries} )

add_executable( simple-draw-with-color simple-draw-with-color )
target_link_libraries( simple-draw-with-color ${libraries} )

add_executable( integrate-glib-main-loop integrate-glib-main-loop )
target_link_libraries( integrate-glib-main-loop ${libraries} )

include_directories( ${CMAKE_SOURCE_DIR} )
set( libraries xdk ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )

add_executable( xdk-simple-window xdk-simple-window )
target_link_libraries( xdk-simple-window ${libraries} )

add_executable( xdk-button xdk-button )
target_link_libraries( xdk-button xdk ${GLIB_LIBRARIES} ${X11_LIBRARIES} )

add_executable( xdk-parent-child xdk-parent-child )
target_link_libraries( xdk-parent-child xdk ${GLIB_LIBRARIES} ${X11_LIBRARIES} )

add_executable( xdk-reparent xdk-reparent )
target_link_libraries( xdk-reparent xdk ${GLIB_LIBRARIES} ${X11_LIBRARIES} )

add_executable( xdk-xcomposite xdk-xcomposite )
target_link_libraries( xdk-xcomposite xdk ${GLIB_LIBRARIES} ${XCOMPOSITE_LIBRARIES} ${X11_LIBRARIES} )

add_executable( xdk-root-struct-redirect xdk-root-struct-redirect )
target_link_libraries( xdk-root-struct-redirect xdk ${GLIB_LIBRARIES} ${XCOMPOSITE_LIBRARIES} ${X11_LIBRARIES} )

add_executable( xdk-tile-wm xdk-tile-wm )
target_link_libraries( xdk-tile-wm xdk ${GLIB_LIBRARIES} ${XCOMPOSITE_LIBRARIES} ${X11_LIBRARIES} )

pkg_check_modules( GTK REQUIRED gtk+-3.0 )
pkg_check_modules( CLUTTER REQUIRED clutter-1.0 )
include_directories( ${GTK_INCLUDE_DIRS} ${CLUTTER_INCLUDE_DIRS} )
link_directories( ${GTK_LIBRARY_DIRS} ${CLUTTER_LIBRARY_DIRS} )
add_executable( clutter-sync clutter-sync )
target_link_libraries( clutter-sync ${GTK_LIBRARIES} ${CLUTTER_LIBRARIES} )

add_executable( gtk-clutter-mixup gtk-clutter-mixup )
target_link_libraries( gtk-clutter-mixup xdk ${GTK_LIBRARIES} ${CLUTTER_LIBRARIES} )

add_executable( xdk-clutter-mixup_c xdk-clutter-mixup.c )
target_link_libraries( xdk-clutter-mixup_c xdk ${CLUTTER_LIBRARIES} )

add_executable( xdk-clutter-xcomposite xdk-clutter-xcomposite )
target_link_libraries( xdk-clutter-xcomposite xdk ${CLUTTER_LIBRARIES} )

add_executable( xdk-rgba-visual xdk-rgba-visual )
target_link_libraries( xdk-rgba-visual xdk ${CLUTTER_LIBRARIES} )
