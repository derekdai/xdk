add_library(
	xdk SHARED
	xdk
	xdk-type
	xdk-type-registry
	xdk-display
	xdk-screen
	xdk-window
	xdk-gc
	xdk-visual
)

target_link_libraries( xdk ${X11_LIBRARIES}  ${GLIB_LIBRARIES} )
