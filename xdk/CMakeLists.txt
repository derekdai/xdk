add_library(
	xdk SHARED
	xdk
	xdk-type-registry
	xdk-display
	xdk-screen
	xdk-window
)

target_link_libraries( xdk ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )
