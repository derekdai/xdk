#include <xdk/xdk.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>

ClutterX11FilterReturn on_event(
	XEvent * xev,
	ClutterEvent * cev,
	XdkDisplay * display)
{
	if(xdk_display_handle_event(display, xev)) {
		CLUTTER_X11_FILTER_REMOVE;
	}
	
	return CLUTTER_X11_FILTER_CONTINUE;
}

int main(int argc, char * args[])
{
	clutter_init(& argc, & args);

	xdk_disable_event_retrieval();
	xdk_set_xdisplay(clutter_x11_get_default_display());
	xdk_init(& argc, & args);
	clutter_x11_add_filter(
		(ClutterX11FilterFunc) on_event,
		xdk_display_get_default());

	ClutterActor * stage = clutter_stage_new();
	clutter_actor_set_reactive(stage, TRUE);
	g_signal_connect(stage, "destroy", G_CALLBACK(clutter_main_quit), NULL);
	clutter_stage_set_user_resizable(CLUTTER_STAGE(stage), TRUE);
	clutter_actor_set_background_color(stage, clutter_color_get_static(CLUTTER_COLOR_TRANSPARENT));
	clutter_actor_show(stage);
	
	clutter_main();
	
	return 0;
}
