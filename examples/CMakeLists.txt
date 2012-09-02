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

#add_executable( xdk-button xdk-button )
#target_link_libraries( xdk-button xdk ${GLIB_LIBRARIES} ${X11_LIBRARIES} )
