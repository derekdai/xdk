#define G_LOG_DOMAIN "XdkParentChild"

#include <xdk/xdk.h>
#include <stdlib.h>

XdkWindow * parent1 = NULL;
XdkWindow * parent2 = NULL;

gboolean on_timeout(XdkWindow * child)
{
	return TRUE;
}

int main(gint argc, gchar * args[])
{
	xdk_init(& argc, & args);
	
	XdkDisplay * display = xdk_display_get_default();
	XdkScreen * screen = xdk_display_get_default_screen(display);
	XdkVisual * visual = xdk_screen_get_rgba_visual(screen);
	
	parent1 = xdk_window_new();
	xdk_window_set_visual(parent1, visual);
	xdk_window_set_background_color(parent1, 0x7f0000ff);
	xdk_window_set_size(parent1, 720, 480);
	xdk_window_show(parent1);

	parent2 = xdk_window_new();
	xdk_window_set_visual(parent2, visual);
	xdk_window_set_background_color(parent2, 0x7f00ff00);
	xdk_window_set_size(parent2, 720, 480);
	xdk_window_set_position(parent2, 1280 - 720, 720 - 480);
	xdk_window_show(parent2);

	xdk_main();
	
	return 0;
}

