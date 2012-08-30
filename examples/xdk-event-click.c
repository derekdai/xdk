#include <xdk/xdk.h>

int main()
{
	 _Xdebug = TRUE;
	
	xdk_init(NULL, NULL);
	
	XdkWindow * win = xdk_get_default_root_window(); //xdk_window_new();
	xdk_window_set_size(win, 1280, 720);
	xdk_window_realize(win);
	g_message("%p, %x", win, xdk_window_get_peer(win));
	//xdk_window_show(win);
	
	/*
	int n_props;
	Atom * atoms = xdk_window_list_properties(win, & n_props);
	for(; n_props > 0; n_props --) {
		char * name = xdk_atom_to_name(atoms[n_props - 1]);
		g_message("%s", name);
		XFree(name);
	}
	XFree(atoms);

	xdk_main();
	*/
	
	return 0;
}
