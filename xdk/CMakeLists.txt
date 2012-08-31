add_definitions( -DG_LOG_DOMAIN="Xdk" )

add_library(
	xdk SHARED
	xdk
	xdk-display
	xdk-screen
	xdk-window
	xdk-gc
	xdk-visual
	xdk-types
)

target_link_libraries( xdk ${X11_LIBRARIES} ${GLIB_LIBRARIES} )
