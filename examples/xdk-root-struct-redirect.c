#include <xdk/xdk.h>

XdkWindow * get_window(Window xid)
{
	XdkDisplay * display = xdk_display_get_default();
	XdkWindow * win = xdk_display_lookup_window(display, xid);
	if(! win) {
		win = xdk_window_new();
		xdk_window_set_foreign_peer(win, xid);
		xdk_window_select_input(win, XDK_EVENT_MASK_STRUCTURE_NOTIFY);
	}
	
	return win;
}

gboolean on_event(XdkWindow * window, XEvent * event)
{
	gboolean result = FALSE;
	
	XdkDisplay * display = xdk_display_get_default();
	
	switch(event->type) {
	case XDK_EVENT_CONFIGURE_REQUEST: {
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
		result = TRUE;
		break;
		}
	case XDK_EVENT_MAP_REQUEST: {
		XMapRequestEvent * e = (XMapRequestEvent *) event;
		XdkWindow * win = get_window(e->window);
		xdk_window_map(win);
		result = TRUE;
		break;
		}
	}
	
	return result;
}

int main()
{
	xdk_init(NULL, NULL);
	
	XdkDisplay * display = xdk_display_get_default();
	
	XdkWindow * root = xdk_get_default_root_window();
	
	xdk_trap_error();
	xdk_window_select_input(root, XDK_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
	xdk_flush();
	
	GError * error = NULL;
	if(xdk_untrap_error(& error)) {
		g_error("Maybe another window manager is running? %s", error->message);
	}
	
	g_signal_connect(root, "event-filter", G_CALLBACK(on_event), NULL);
	
	xdk_main();

	return 0;
}
