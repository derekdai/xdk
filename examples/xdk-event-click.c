#include <xdk/xdk.h>

int main()
{
	 _Xdebug = TRUE;
	
	xdk_init(NULL, NULL);
	
	XdkWindow * win = xdk_window_new();
	xdk_window_set_size(win, 1280, 720);
	xdk_window_show(win);
	g_signal_connect(win, "destroy", G_CALLBACK(xdk_main_quit), NULL);
	
	int n_props;
	Atom * atoms = xdk_window_list_properties(win, & n_props);
	for(; n_props > 0; n_props --) {
		char * name = xdk_atom_to_name(atoms[n_props - 1]);
		g_message("%s", name);
		XFree(name);
	}
	XFree(atoms);

	xdk_main();
	
	return 0;
}
