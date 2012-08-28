#include "xdk.h"

int main(int * argc, char * args[])
{
	if(! xdk_init(NULL, NULL)) {
		g_error("Failed to init XDK");
	}
	
	XdkDisplay * display = xdk_display_get_default();
	g_message("Default display '%s' has %d screens, vendor is %s, version %d",
		xdk_display_get_name(display),
		xdk_display_get_n_screens(display),
		xdk_display_get_vendor(display),
		xdk_display_get_release(display));
		
	XdkScreen * screen = xdk_display_get_default_screen(display);
	g_message("Default screen number is %d (%d x %d), %d dpp",
		xdk_screen_get_number(screen),
		xdk_screen_get_width(screen),
		xdk_screen_get_height(screen),
		xdk_screen_get_default_depth(screen));
		
	XdkWindow * win = xdk_window_new();
	xdk_window_set_position(win, 100, 100);
	xdk_window_set_size(win, 1280, 720);
	xdk_window_realize_simple(win);
	xdk_window_map(win);
	
	xdk_display_flush(display);
	
	g_usleep(3 * 1000 * 1000);
	
	return 0;
}
