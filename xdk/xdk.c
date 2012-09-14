#include "xdk.h"

static GMainLoop * loop = NULL;

static gboolean event_retrieval_disabled = FALSE;

void xdk_disable_event_retrieval()
{
	event_retrieval_disabled = TRUE;
}

void xdk_init(int * argc, char ** args[])
{
	g_type_init();
	
	static volatile gsize initialized = FALSE;
	if(g_once_init_enter(& initialized)) {
		loop = g_main_loop_new(NULL, FALSE);
		
		if(! event_retrieval_disabled) {
			XdkDisplay * display = xdk_display_get_default();
			xdk_display_add_watch(display);
		}
		
		g_once_init_leave(& initialized, TRUE);
	}
}

void xdk_main()
{
	g_main_loop_run(loop);
}

void xdk_main_quit()
{
	g_main_loop_quit(loop);
}
