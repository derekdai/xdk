#include <xdk/xdk.h>

XdkWindow * root;

XdkWindow * get_foreign_window(Window xid)
{
	XdkDisplay * display = xdk_display_get_default();
	XdkWindow * win = xdk_display_lookup_window(display, xid);
	if(! win) {
		win = xdk_window_new();
		xdk_window_set_foreign_peer(win, xid);
		xdk_window_select_input(win, XDK_EVENT_MASK_STRUCTURE_NOTIFY | XDK_EVENT_MASK_STRUCTURE_NOTIFY);
	}
	
	return win;
}

gboolean on_event(XdkDisplay * display, XEvent * event, XdkWindow * root)
{
	switch(event->type) {
	case XDK_EVENT_CONFIGURE_REQUEST:
	case XDK_EVENT_MAP_REQUEST:
	case XDK_EVENT_RESIZE_REQUEST:
	case XDK_EVENT_CIRCULATE_REQUEST:
	case XDK_EVENT_DESTROY: {
		event->xany.window = xdk_window_get_peer(root);
		break;
		}
	}
	
	return FALSE;
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
}

void on_map_request(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	XMapRequestEvent * e = (XMapRequestEvent *) event;
	XdkWindow * win = get_foreign_window(e->window);
	xdk_window_map(win);
}

void on_resize_request(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
}

void on_destroy_notify(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	XDestroyWindowEvent * e = (XDestroyWindowEvent *) event;
	XdkWindow * win = xdk_display_lookup_window(display, e->window);
	if(win) {
		xdk_window_destroy(win);
		g_object_unref(win);
	}
}

int main()
{
	xdk_init(NULL, NULL);
	
	XdkDisplay * display = xdk_display_get_default();
	XdkWindow * root = xdk_get_default_root_window();

	xdk_display_add_event_filter(display, (XdkEventFilter) on_event, root);
	
	xdk_trap_error();
	xdk_window_select_input(root, XDK_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
	xdk_flush();
	
	GError * error = NULL;
	if(xdk_untrap_error(& error)) {
		g_error("Maybe another window manager is running? %s", error->message);
	}
	
	g_signal_connect(root, "configure-request", G_CALLBACK(on_configure_request), display);
	g_signal_connect(root, "map-request", G_CALLBACK(on_map_request), display);
	g_signal_connect(root, "resize-request", G_CALLBACK(on_resize_request), display);
	g_signal_connect(root, "destroy-notify", G_CALLBACK(on_destroy_notify), display);
	
	xdk_main();

	return 0;
}
