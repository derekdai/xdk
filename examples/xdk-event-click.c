#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif
#define G_LOG_DOMAIN	"XdkEventClick"

#include <xdk/xdk.h>

void on_destroy()
{
	g_message("Bye");
	
	xdk_main_quit();
}

gboolean on_event(XdkDisplay * display, XEvent * event)
{
	xdk_util_event_dump(event);
	
	return TRUE;
}

int main()
{
	xdk_init(NULL, NULL);
	
	XdkDisplay * display = xdk_display_get_default();
	xdk_display_set_event_filter(display, on_event);
	
	XdkWindow * win = xdk_window_new();
	xdk_window_set_size(win, 1280, 720);
	xdk_window_show(win);
	g_signal_connect(win, "destroy", G_CALLBACK(on_destroy), NULL);
	
	int n_props;
	Atom * atoms = xdk_window_list_properties(win, & n_props);
	for(; n_props > 0; n_props --) {
		char * name = xdk_atom_to_name(atoms[n_props - 1]);
		g_message("Window property: %s", name);
		XFree(name);
	}
	XFree(atoms);
	
	xdk_main();
	
	return 0;
}
