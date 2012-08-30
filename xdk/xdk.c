#include "xdk.h"

static gboolean quited = FALSE;

gboolean xdk_init(int * argc, char ** args[])
{
	g_type_init();
	
	return xdk_display_init_once();
}

void xdk_main()
{
	XdkDisplay * display = xdk_display_get_default();
	
	while(! quited && xdk_display_next_event(display)) {
		xdk_display_dispatch_event(display);
	}
}

void xdk_main_quit()
{
	quited = TRUE;
}
