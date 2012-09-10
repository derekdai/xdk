#include <xdk/xdk.h>

XdkWindow * root;

#define WIDTH (1280)
#define HEIGHT (720)
#define COLS (4)
#define ROWS (3)
#define CELL_WIDTH (WIDTH/COLS)
#define CELL_HEIGHT (HEIGHT/ROWS)

GList * windows = NULL;

XdkWindow * get_foreign_window(Window xid, gboolean create)
{
	XdkDisplay * display = xdk_display_get_default();
	XdkWindow * win = xdk_display_lookup_window(display, xid);
	if(! win && create) {
		win = xdk_window_new();
		xdk_window_set_foreign_peer(win, xid);
		xdk_window_select_input(win, XDK_EVENT_MASK_STRUCTURE_NOTIFY | XDK_EVENT_MASK_STRUCTURE_NOTIFY);
	}
	
	return win;
}

gboolean on_event(XdkDisplay * display, XEvent * event, XdkWindow * root)
{
	switch(event->type) {
	case XDK_EVENT_UNMAP:
	case XDK_EVENT_CONFIGURE_REQUEST:
	case XDK_EVENT_MAP_REQUEST:
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
	XdkWindow * win = get_foreign_window(e->window, TRUE);
	if(xdk_window_has_override_redirect(win)) {
		return;
	}
	
	xdk_window_set_size(win, CELL_WIDTH, CELL_HEIGHT);
}

void on_map_request(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	XMapRequestEvent * e = (XMapRequestEvent *) event;
	XdkWindow * win = get_foreign_window(e->window, TRUE);
	if(xdk_window_has_override_redirect(win)) {
		return;
	}
	if(g_list_find(windows, win)) {
		g_message("Already mapped");
		return;
	}
	
	windows = g_list_append(windows, win);
	
	int col, row;
	GList * node = g_list_first(windows);
	for(row = 0; row < ROWS; row ++) {
		for(col = 0; col < COLS; col ++) {
			if(NULL == node) {
				return;
			}
			
			win = XDK_WINDOW(node->data);
			xdk_window_set_position(win, col * CELL_WIDTH, row * CELL_HEIGHT);
			xdk_window_map(win);
			
			node = g_list_next(node);
		}
	}
}

void on_unmap_notify(XdkWindow * root, XEvent * event, XdkDisplay * display)
{
	XUnmapEvent * e = (XUnmapEvent *) event;
	XdkWindow * win = get_foreign_window(e->window, FALSE);
	if(! win) {
		return;
	}
	if(xdk_window_has_override_redirect(win)) {
		return;
	}
	
	xdk_window_unmap(win);
	GList * node = g_list_find(windows, win);
	windows = g_list_delete_link(windows, node);
	g_message("Unmap: %d", g_list_length(windows));
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
	g_signal_connect(root, "unmap-notify", G_CALLBACK(on_unmap_notify), display);
	g_signal_connect(root, "destroy-notify", G_CALLBACK(on_destroy_notify), display);
	
	xdk_main();

	return 0;
}
