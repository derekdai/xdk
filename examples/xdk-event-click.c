#include <xdk/xdk.h>

int main()
{
	 _Xdebug = TRUE;
	
	xdk_init(NULL, NULL);
	
	XdkWindow * win = xdk_get_default_root_window();
	
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
