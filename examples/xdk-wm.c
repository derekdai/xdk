#include <xdk/xdk.h>

void on_screen_add()
{
	g_message("Screen added");
}

void on_screen_remove()
{
	g_message("Screen removed");
}

void on_window_add(XdkWindowManager * wm, XdkWindow * win)
{
	g_message("Window added: %lu %s %s",
		xdk_window_get_peer(win),
		xdk_window_is_maximized(win) ? "true" : "false",
		xdk_window_get_visible(win) ? "true" : "false");
}

void on_window_remove()
{
	g_message("Window removed");
}

int main(gint argc, gchar * args[])
{
	xdk_init(& argc, & args);
	
	XdkDisplay * display = xdk_display_get_default();
	XdkScreen * screen = xdk_display_get_default_screen(display);
	XdkWindow * root = xdk_screen_get_root_window(screen);
	
	/*
	XdkWindow * win = xdk_window_new();
	xdk_window_show(win);
	xdk_window_set_size(win, 720, 480);
	xdk_display_flush(display);
	*/

	GError * error = NULL;
	XdkWindowManager * wm = xdk_window_manager_new();
	g_signal_connect(wm, "screen-added", G_CALLBACK(on_screen_add), NULL);
	g_signal_connect(wm, "screen-removed", G_CALLBACK(on_screen_remove), NULL);
	g_signal_connect(wm, "window-added", G_CALLBACK(on_window_add), NULL);
	g_signal_connect(wm, "window-removed", G_CALLBACK(on_window_remove), NULL);
	if(! xdk_window_manager_add_all_screens(wm, NULL, & error)) {
		if(error) {
			g_error("%s", error->message);
		}
	}
	
	xdk_main();
	
	g_object_unref(wm);
	
	return 0;
}
