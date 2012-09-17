#include <xdk/xdk.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>

ClutterActor * overlay = NULL;

GHashTable * win_actor_map = NULL;

ClutterX11FilterReturn on_event(
	XEvent * xev,
	ClutterEvent * cev,
	XdkDisplay * display)
{
	ClutterX11FilterReturn result = CLUTTER_X11_FILTER_CONTINUE;
	switch(xev->type) {
	case ConfigureNotify:
		
		break;
	case DestroyNotify:
		xev->xany.window = xdk_window_get_peer(xdk_get_default_root_window());
		result = CLUTTER_X11_FILTER_REMOVE;
		break;
	}
	
	xdk_display_handle_event(display, xev);
	
	return result;
}

ClutterActor * lookup_actor(Window window, gboolean create)
{
	ClutterActor * actor = g_hash_table_lookup(win_actor_map, GUINT_TO_POINTER(window));
	
	if(! actor && create) {
		actor = clutter_rectangle_new_with_color(clutter_color_get_static(CLUTTER_COLOR_GREEN));

		actor = clutter_x11_texture_pixmap_new_with_window(window);
		clutter_x11_texture_pixmap_set_automatic(actor, TRUE);
		clutter_actor_hide(actor);
		clutter_actor_add_child(overlay, actor);

		g_object_set_data(actor, "window", GUINT_TO_POINTER(window));
		g_hash_table_insert(win_actor_map, GUINT_TO_POINTER(window), actor);
	}
	
	return actor;
}

void remove_actor(Window window)
{
	g_hash_table_remove(win_actor_map, window);
}

void on_configure_request(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	XConfigureRequestEvent * e = (XConfigureRequestEvent *) event;
	XWindowChanges change = {
		.x = e->x,
		.y = e->y,
		.width = e->width,
		.height = e->height,
		.border_width = e->border_width,
		.sibling = e->above,
		.stack_mode = e->detail
	};
	XConfigureWindow(
		xdk_display_get_peer(display),
		e->window,
		e->value_mask,
		& change);
	
	ClutterActor * actor = lookup_actor(e->window, TRUE);
	clutter_actor_set_position(actor, e->x, e->y);
	clutter_actor_set_size(actor, e->width, e->height);
	if(None != e->above) {
		ClutterActor * sibling = lookup_actor(e->above, TRUE);
		clutter_actor_set_child_above_sibling(
			clutter_actor_get_parent(actor),
			actor,
			sibling);
	}
}

void on_map_request(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	XMapRequestEvent * e = (XMapRequestEvent *) event;
	xdk_trap_error();
	XMapWindow(xdk_display_get_peer(display), e->window);
	ClutterActor * actor = lookup_actor(e->window, TRUE);
	clutter_actor_show(actor);
}

void on_resize_request(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	g_message("on_resize_request");
}

void on_unmap_notify(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	XUnmapEvent * e = (XUnmapEvent *) event;
	ClutterActor * actor = lookup_actor(e->window, FALSE);
	if(actor) {
		clutter_actor_hide(actor);
	}
	XUnmapWindow(xdk_display_get_peer(display), e->window);
}

void on_destroy_notify(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	remove_actor(((XDestroyWindowEvent *) event)->window);
}

gint xcomp_event_base = 0, xcomp_error_base = 0;

gint main(gint argc, gchar * args[])
{
	win_actor_map = g_hash_table_new_full(
		g_direct_hash, g_direct_equal,
		NULL, (GDestroyNotify) clutter_actor_destroy);
	
	clutter_x11_set_use_argb_visual(TRUE);
	if(CLUTTER_INIT_SUCCESS != clutter_init(& argc, & args)) {
		return 1;
	}
	
	xdk_disable_event_retrieval();
	xdk_set_xdisplay(clutter_x11_get_default_display());
	xdk_init(& argc, & args);

	XdkDisplay * display = xdk_display_get_default();
	XdkWindow * root = xdk_get_default_root_window();

	// check if there is a composite manager exists, if not, we are the one
	Atom net_wm_cm = xdk_display_atom_from_name(display, "_NET_WM_CM_S0", FALSE);
	Window cm = XGetSelectionOwner(xdk_display_get_peer(display), net_wm_cm);
	if(None != cm) {
		g_error("Another composite manager exists");
	}

	XdkWindow * cm_holder = xdk_window_new();
	xdk_window_realize(cm_holder);
	XSetSelectionOwner(xdk_display_get_peer(display), net_wm_cm, xdk_window_get_peer(cm_holder), 0);
	
	// check if window manage exists. if not, redirect events
	xdk_trap_error();
	xdk_window_select_input(
		root,
		XDK_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
		XDK_EVENT_MASK_STRUCTURE_NOTIFY |
		XDK_EVENT_MASK_VISIBILITY_CHANGE);
	xdk_flush();
	
	GError * error = NULL;
	if(xdk_untrap_error(& error)) {
		g_error("Maybe another window manager is running? %s", error->message);
	}

	overlay = clutter_stage_new();
	clutter_stage_set_use_alpha(overlay, TRUE);

	// list all windows and create coresponding actors
	xdk_display_grab_server(display);
	GList * windows = xdk_window_query_tree(root);
	GList * node = windows;
	while(node) {
		//xdk_util_window_dump((Window) node->data);
		Window w = (Window) node->data;
		XWindowAttributes attrs;
		XGetWindowAttributes(
			xdk_display_get_peer(display),
			w, & attrs);
		if(IsUnmapped == attrs.map_state) {
			node = g_list_next(node);
			continue;
		}
		
		ClutterActor * actor = lookup_actor(w, TRUE);
		clutter_actor_set_position(actor, attrs.x, attrs.y);
		clutter_actor_set_size(actor, attrs.width, attrs.height);
		clutter_actor_show(actor);
		
		node = g_list_next(node);
	}

	g_list_free(windows);
	xdk_display_ungrab_server(display);
	
	g_signal_connect(root, "configure-request", G_CALLBACK(on_configure_request), display);
	g_signal_connect(root, "map-request", G_CALLBACK(on_map_request), display);
	g_signal_connect(root, "unmap-notify", G_CALLBACK(on_unmap_notify), display);
	g_signal_connect(root, "destroy-notify", G_CALLBACK(on_destroy_notify), display);

	// every checks passed, show the composite overlay and do the compositing
	Window xoverlay = XCompositeGetOverlayWindow(xdk_display_get_peer(display), xdk_window_get_peer(root));
	// event passthrough mode
	XserverRegion region = XFixesCreateRegion (xdk_display_get_peer(display), NULL, 0);
	XFixesSetWindowShapeRegion (xdk_display_get_peer(display), xoverlay, ShapeBounding, 0, 0, 0);
	XFixesSetWindowShapeRegion (xdk_display_get_peer(display), xoverlay, ShapeInput, 0, 0, region);
	XFixesDestroyRegion (xdk_display_get_peer(display), region);
	
	// create stage on top of overlay
	int x, y, width, height, border_width, depth;
	Window r;
	if(! XGetGeometry(xdk_display_get_peer(display), xoverlay, & x, & x, & y, & width, & height, & border_width, & depth)) {
		g_error("Failed to get overlay geometry");
	}
	clutter_actor_set_position(overlay, x, y);
	clutter_actor_set_size(overlay, width, height);
	clutter_actor_realize(overlay);
	XReparentWindow(xdk_display_get_peer(display), clutter_x11_get_stage_window(overlay), xoverlay, 0, 0);
	clutter_actor_show(overlay);
	
	// register event filter for Xdk
	clutter_x11_add_filter((ClutterX11FilterFunc) on_event, display);
	
	clutter_main();
	
	return 0;
}
