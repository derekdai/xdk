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
	xdk-util
	xdk-window-manager
)

target_link_libraries(
	xdk
	${XCOMPOSITE_LIBRARIES}
	${XDAMAGE_LIBRARIES}
	${X11_LIBRARIES}
	${GLIB_LIBRARIES}
)

install( TARGETS xdk LIBRARY DESTINATION lib )
