#include "xdk.h"

static GMainLoop * loop = NULL;

gboolean xdk_init(int * argc, char ** args[])
{
	g_type_init();
	
	if(! loop) {
		XdkDisplay * display = xdk_display_get_default();
		xdk_display_add_watch(display);
		loop = g_main_loop_new(NULL, FALSE);
	}
	
	return;
}

void xdk_main()
{
	g_main_loop_run(loop);
}

void xdk_main_quit()
{
	g_main_loop_quit(loop);
}
