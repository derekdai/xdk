#include <xdk/xdk.h>

void on_destroy()
{
	g_message("Bye");
	
	xdk_main_quit();
}

void on_disconnect()
{
	g_message("Disconnected");
}

gboolean on_event(XdkDisplay * display, XEvent * event)
{
	xdk_util_event_dump(event);
	
	return TRUE;
}

void on_timeout(XdkWindow * win)
{
	XdkDisplay * display = xdk_display_get_default();
	XDestroyWindow(xdk_display_get_peer(display), xdk_window_get_peer(win));
	XFlush(xdk_display_get_peer(display));
	g_message("Destroyed: %u", xdk_window_get_peer(win));
}

int main()
{
	//_Xdebug = TRUE;
	
	xdk_init(NULL, NULL);
	
	XdkDisplay * display = xdk_display_get_default();
	xdk_display_set_event_filter(display, on_event);
	g_signal_connect(display, "disconnect", G_CALLBACK(on_disconnect), NULL);
	
	XdkWindow * win = xdk_window_new();
	xdk_window_set_size(win, 1280, 720);
	xdk_window_show(win);
	g_signal_connect(win, "destroy", G_CALLBACK(on_destroy), NULL);
	
	int n_props;
	Atom * atoms = xdk_window_list_properties(win, & n_props);
	for(; n_props > 0; n_props --) {
		char * name = xdk_atom_to_name(atoms[n_props - 1]);
		g_message("%s", name);
		XFree(name);
	}
	XFree(atoms);
	
	//g_timeout_add_seconds(3, (GSourceFunc) on_timeout, win);

	xdk_main();
	
	return 0;
}
