add_executable( simple-window simple-window )
target_link_libraries( simple-window ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )

add_executable( simple-window-keypress-draw simple-window-keypress-draw )
target_link_libraries( simple-window-keypress-draw ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )

add_executable( simple-draw-with-color simple-draw-with-color )
target_link_libraries( simple-draw-with-color ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )

add_executable( xdk-simple-window xdk-simple-window )
target_link_libraries( xdk-simple-window xdk ${X11_LIBRARIES} ${GLIB_LIBRARIES} )
