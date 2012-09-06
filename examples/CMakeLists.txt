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

add_executable( xdk-root-events xdk-root-events )
target_link_libraries( xdk-root-events xdk ${GLIB_LIBRARIES} ${X11_LIBRARIES} )

add_executable( xdk-xcomposite xdk-xcomposite )
target_link_libraries( xdk-xcomposite xdk ${GLIB_LIBRARIES} ${XCOMPOSITE_LIBRARIES} ${X11_LIBRARIES} )

pkg_check_modules( CLUTTER REQUIRED clutter-1.0 )
include_directories( ${CLUTTER_INCLUDE_DIRS} )
link_directories( ${CLUTTER_LIBRARY_DIRS} )
add_executable( composite composite )
target_link_libraries( composite xdk ${CLUTTER_LIBRARIES} )

pkg_check_modules( GTK REQUIRED gtk+-3.0 )
include_directories( ${GTK_INCLUDE_DIRS} )
link_directories( ${GTK_LIBRARY_DIRS} )
add_executable( clutter-sync clutter-sync )
target_link_libraries( clutter-sync ${GTK_LIBRARIES} ${CLUTTER_LIBRARIES} )
