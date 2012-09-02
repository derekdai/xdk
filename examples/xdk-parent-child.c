#define G_LOG_DOMAIN "XdkParentChild"

#include <xdk/xdk.h>

int main(gint argc, gchar * args[])
{
	xdk_init(& argc, & args);
	
	XdkWindow * parent = xdk_window_new();
	xdk_window_set_size(parent, 320, 240);
	g_signal_connect(parent, "destroy", G_CALLBACK(xdk_main_quit), NULL);
	XdkWindow * child = xdk_window_new();
	xdk_window_set_background_color(child, 0x00ff00);
	xdk_window_set_position(child, 50, 50);
	xdk_window_set_size(child, 50, 50);
	xdk_window_add_child(parent, child);
	//xdk_window_show(child);
	g_object_unref(child);
	//xdk_window_show(parent);
	xdk_window_show_all(parent);
	
	xdk_main();
	
	g_object_unref(parent);
	
	return 0;
}
