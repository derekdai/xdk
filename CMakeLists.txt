cmake_minimum_required( VERSION 2.8 )

project( test-xlib )

find_package( PkgConfig )

pkg_check_modules( X11 REQUIRED x11 )
pkg_check_modules( GLIB REQUIRED glib-2.0 )

include_directories(
	${CMAKE_SOURCE_DIR}/xdk
	${X11_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
)

link_directories( ${X11_LIBRARY_DIRS} ${GLIB_LIBRARY_DIRS} )

add_subdirectory( xdk )

add_subdirectory( examples )
