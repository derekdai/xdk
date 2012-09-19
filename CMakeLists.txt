cmake_minimum_required( VERSION 2.8 )

project( test-xlib )

find_package( PkgConfig )

pkg_check_modules( X11 REQUIRED x11 )
pkg_check_modules( GLIB REQUIRED gobject-2.0 )
pkg_check_modules( XCOMPOSITE REQUIRED xcomposite )
pkg_check_modules( XDAMAGE REQUIRED xdamage )

include_directories(
	${CMAKE_SOURCE_DIR}/xdk
	${XCOMPOSITE_INCLUDE_DIRS}
	${XDAMAGE_INCLUDE_DIRS}
	${X11_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
)

link_directories(
	${XCOMPOSITE_LIBRARY_DIRS}
	${XDAMAGE_LIBRARY_DIRS}
	${X11_LIBRARY_DIRS}
	${GLIB_LIBRARY_DIRS}
)

add_subdirectory( xdk )

add_subdirectory( examples )
