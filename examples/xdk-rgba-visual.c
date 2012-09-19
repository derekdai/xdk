#include <xdk/xdk.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <X11/extensions/Xrender.h>

/**
 * 
 * Set visual different from parent window must set
 * 1. depth
 * 2. visual
 * 3. colormap (create with visual in step 2)
 * 4. border pixel
 * 5. background pixel
 * Or bad match error results. See
 *   http://stackoverflow.com/questions/3645632/how-to-create-a-window-with-a-bit-depth-of-32
 *   http://cgit.freedesktop.org/xorg/xserver/tree/dix/window.c#n615
 * 
 */
int main(int argc, char * args[])
{
	clutter_x11_set_use_argb_visual(TRUE);
	clutter_init(& argc, & args);
	
	xdk_disable_event_retrieval();
	xdk_set_xdisplay(clutter_x11_get_default_display());
	xdk_init(& argc, & args);
	
	XdkDisplay * display = xdk_display_get_default();
	XdkScreen * screen = xdk_display_get_default_screen(display);
	XdkVisual * visual = xdk_screen_get_rgba_visual(screen);
	
	XdkWindow * window = xdk_window_new();
	xdk_window_set_size(window, 720, 480);
	xdk_window_set_visual(window, visual);
	xdk_window_set_background_color(window, 0x7fff0000);
	xdk_window_realize(window);
	
	ClutterActor * stage = clutter_stage_new();
	clutter_x11_set_stage_foreign(CLUTTER_STAGE(stage), xdk_window_get_peer(window));
	clutter_stage_set_use_alpha(CLUTTER_STAGE(stage), TRUE);
	clutter_actor_set_background_color(stage, clutter_color_get_static(CLUTTER_COLOR_TRANSPARENT));
	g_signal_connect(stage, "destroy", G_CALLBACK(clutter_main_quit), NULL);
	
	xdk_window_show(window);
	
	clutter_main();

	/*
	Display * display = XOpenDisplay(NULL);
	if(! display) {
		g_error("Failed to open display");
	}
	Screen * screen = DefaultScreenOfDisplay(display);
	Window root = DefaultRootWindow(display);
	
	//XVisualInfo visual_info;
	//if(! XMatchVisualInfo(display, screen, 32, TrueColor, & visual_info)) {
	//	g_error("No RGBA visual found");
	//}
	
	XVisualInfo template = {
		.screen = 0,
		.depth = 32,
		.class = TrueColor,
		.red_mask = 0xff0000,
		.green_mask = 0xff00,
		.blue_mask = 0xff,
	};
	int n_visual_infos;
	XVisualInfo * visual_infos = XGetVisualInfo(
		display,
		VisualScreenMask | VisualDepthMask | VisualClassMask |
			VisualRedMaskMask | VisualGreenMaskMask | VisualBlueMaskMask,
		& template,
		& n_visual_infos);
	if(! visual_infos) {
		g_error("No RGBA visual found");
	}
	XVisualInfo visual_info = * visual_infos;
	XFree(visual_infos);

	Colormap colormap = XCreateColormap(display, root, visual_info.visual, AllocNone);
	if(None == colormap) {
		g_error("Failed to create colormap");
	}
	
	XSetWindowAttributes attrs = {
		.background_pixel = 0x7fff0000,
		.border_pixel = 0,
		.colormap = colormap,
	};
	Window win = XCreateWindow(
		display, root,
		0, 0, 720, 480,
		0,							// border width
		visual_info.depth,			// depth
		InputOutput,				// class
		visual_info.visual,			// visual
		CWBackPixel | CWBorderPixel | CWColormap, // valuemask
		& attrs); //& attrs);
	if(None == win) {
		g_error("Failed to create window");
	}
	XMapWindow(display, win);
	XFlush(display);
	
	g_usleep(3 * G_USEC_PER_SEC);
	/*

	return 0;
}
