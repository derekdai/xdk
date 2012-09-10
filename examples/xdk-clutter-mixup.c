#include <xdk/xdk.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>

gboolean on_event(XdkDisplay * display, XEvent * event)
{
	if(CLUTTER_X11_FILTER_CONTINUE == clutter_x11_handle_event(event)) {
		return FALSE;
	}
	
	return TRUE;
}

int main(int argc, char * args[])
{
	xdk_init(& argc, & args);
	XdkDisplay * display = xdk_display_get_default();
	clutter_x11_disable_event_retrieval();
	clutter_x11_set_display(xdk_display_get_peer(display));
	clutter_init(& argc, & args);
	
	xdk_display_add_event_filter(display, (XdkEventFilter) on_event, NULL);
	
	ClutterActor * stage = clutter_stage_new();
	g_signal_connect(stage, "destroy", G_CALLBACK(xdk_main_quit), NULL);
	clutter_actor_show(stage);
	
	xdk_main();
	
	return 0;
}

