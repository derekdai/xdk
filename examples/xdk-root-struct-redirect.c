#include <xdk/xdk.h>

gboolean on_event(XdkWindow * window, XEvent * event)
{
	gboolean result = TRUE;
	
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
			xdk_display_get_peer(xdk_display_get_default()),
			e->window,
			e->value_mask,
			& change);
		XMapWindow(
			xdk_display_get_peer(xdk_display_get_default()),
			e->window);
		result = FALSE;
		break;
		}
	case XDK_EVENT_MAP_REQUEST: {
		XMapRequestEvent * e = (XMapRequestEvent *) event;
		XMapWindow(
			xdk_display_get_peer(xdk_display_get_default()),
			e->window);
		result = FALSE;
		break;
		}
	}
	
	if(! result) {
		xdk_flush();
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
