#include "xdk.h"
#include "xdk-display-private.h"

static GMainLoop * loop = NULL;

static gboolean event_retrieval_disabled = FALSE;

static Display * foreign_display = NULL;

volatile XdkDisplay * xdk_default_display;

void xdk_set_xdisplay(Display * display)
{
	g_return_if_fail(display);
	
	foreign_display = display;
}

void xdk_disable_event_retrieval()
{
	event_retrieval_disabled = TRUE;
}

void xdk_init(int * argc, char ** args[])
{
	g_type_init();
	
	if(g_once_init_enter(& xdk_default_display)) {
		loop = g_main_loop_new(NULL, FALSE);
		
		XdkDisplay * display = g_object_new(
			XDK_TYPE_DISPLAY,
			"peer", foreign_display,
			"event-retreival-disabled", event_retrieval_disabled,
			NULL);
		if(! xdk_display_open(display)) {
			g_error("Failed to open display");
		}
		
		g_once_init_leave(& xdk_default_display, display);
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
