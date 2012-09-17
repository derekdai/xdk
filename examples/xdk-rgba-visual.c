#include <xdk/xdk.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>

int main(int argc, char * args[])
{
	clutter_x11_set_use_argb_visual(TRUE);
	if(CLUTTER_INIT_SUCCESS != clutter_init(& argc, & args)) {
		g_error("Failed to init clutter");
	}
	
	xdk_disable_event_retrieval();
	xdk_set_xdisplay(clutter_x11_get_default_display());
	xdk_init(& argc, & args);
	
	XdkWindow * window = xdk_window_new();
	xdk_window_set_size(window, 720, 480);
	xdk_window_realize(window);
	
	XdkScreen * screen = xdk_get_default_screen();
	XdkVisual * visual = xdk_screen_get_rgba_visual(screen);
	g_message("%d, %lx, %lx, %lx",
		xdk_visual_get_depth(visual),
		xdk_visual_get_red_mask(visual),
		xdk_visual_get_green_mask(visual),
		xdk_visual_get_blue_mask(visual));
	
	ClutterActor * stage = clutter_stage_new();
	clutter_x11_set_stage_foreign(CLUTTER_STAGE(stage), xdk_window_get_peer(window));
	clutter_stage_set_use_alpha(CLUTTER_STAGE(stage), TRUE);
	clutter_actor_set_background_color(stage, clutter_color_get_static(CLUTTER_COLOR_TRANSPARENT));
	g_signal_connect(stage, "destroy", G_CALLBACK(clutter_main_quit), NULL);
	
	xdk_window_show(window);
	
	clutter_main();
	
	return 0;
}
