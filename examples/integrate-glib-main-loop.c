#include <glib-object.h>
#include <X11/Xlib.h>

#define X_TYPE_EVENT (x_event_get_type())

#define MY_TYPE_WIN (my_win_get_type())
#define MY_WIN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MY_TYPE_WIN, MyWin))

typedef struct _MyWinClass MyWinClass;

typedef struct _MyWin MyWin;

enum _MyWinSignal
{
	MY_WIN_SIGNAL_CONFIGURE_NOTIFY,
	MY_WIN_SIGNAL_DELETE_EVENT,
	MY_WIN_SIGNAL_LAST
};

struct _MyWinClass
{
	GObjectClass base;
	
	void (* delete_event)(MyWin * self, XEvent * event);
	
	void (* configure_notify)(MyWin * self, XEvent * event);
};

struct _MyWin
{
	GObject base;
	
	Window peer;
	
	gint x;
	
	gint y;
	
	guint width;
	
	guint height;
};

void my_win_handle_delete_event(MyWin * self, XEvent * event);

void my_win_handle_configure_notify(MyWin * self, XEvent * event);

void my_win_finalize(GObject * object);

G_DEFINE_TYPE(MyWin, my_win, G_TYPE_OBJECT);

Display * display;

GHashTable * windows;

guint my_win_signals[MY_WIN_SIGNAL_LAST];

GMainLoop * loop;

Atom ATOM_WM_DELETE_WINDOW;

XEvent * x_event_copy(const XEvent * event)
{
	return g_slice_dup(XEvent, event);
}

void x_event_free(XEvent * event)
{
	g_slice_free(XEvent, event);
}

GType x_event_get_type()
{
	static GType type = 0;
	if(! type) {
		type = g_boxed_type_register_static(
			"XEvent",
			(GBoxedCopyFunc) x_event_copy,
			(GBoxedFreeFunc) x_event_free);
	}
	
	return type;
}

void my_win_class_init(MyWinClass * clazz)
{
	GObjectClass * object_class = G_OBJECT_CLASS(clazz);
	object_class->finalize = my_win_finalize;
	
	clazz->delete_event = my_win_handle_delete_event;
	clazz->configure_notify = my_win_handle_configure_notify;
	
	my_win_signals[MY_WIN_SIGNAL_DELETE_EVENT] = g_signal_new(
		"delete-event",
		G_TYPE_FROM_CLASS(clazz),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(MyWinClass, delete_event),
		NULL, NULL,
		g_cclosure_marshal_generic,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	my_win_signals[MY_WIN_SIGNAL_CONFIGURE_NOTIFY] = g_signal_new(
		"configure-notify",
		G_TYPE_FROM_CLASS(clazz),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(MyWinClass, configure_notify),
		NULL, NULL,
		g_cclosure_marshal_generic,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
}

void my_win_init(MyWin * self)
{
	self->peer = None;
	self->width = 1280;
	self->height = 720;
}

void my_win_realize(MyWin * self)
{
	if(None != self->peer) {
		return;
	}
	
	Window root = DefaultRootWindow(display);
	if(None == root) {
		g_error("No root window found");
	}
	
	XSetWindowAttributes attributes = {
		.background_pixel = 0x7fffffff,
		.event_mask = StructureNotifyMask
	};
	self->peer = XCreateWindow(
		display, root,
		self->x, self->y, self->width, self->height,
		0,												/* border width */
		CopyFromParent,									/* depth */
		InputOutput,									/* class */
		CopyFromParent,									/* visual */
		CWBackPixel | CWEventMask, & attributes);
	if(None == self->peer) {
		g_error("Failed to create window");
	}
	XMapWindow(display, self->peer);

	/* see simple-window-wm-delete-window */
	Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", FALSE);
	XSetWMProtocols(display, self->peer, & wm_delete_window, 1);
	
	g_hash_table_insert(windows, GUINT_TO_POINTER(self->peer), self);
}

void my_win_destroy(MyWin * self)
{
	if(None == self->peer) {
		return;
	}
	
	g_hash_table_remove(windows, GUINT_TO_POINTER(self->peer));
	
	XDestroyWindow(display, self->peer);
	XFlush(display);
	self->peer = None;
}

void my_win_finalize(GObject * object)
{
	my_win_destroy(MY_WIN(object));
	
	G_OBJECT_CLASS(my_win_parent_class)->finalize(object);
}

void my_win_handle_delete_event(MyWin * self, XEvent * event)
{
	// destroy if we receive delete event sent by window manager
	my_win_destroy(self);
}

void my_win_handle_configure_notify(MyWin * self, XEvent * event)
{
	self->x = event->xconfigure.x;
	self->y = event->xconfigure.y;
	self->width = event->xconfigure.width;
	self->height = event->xconfigure.height;
}

void my_win_dispatch_event(MyWin * self, XEvent * event)
{
	guint signal_id;
	switch(event->type) {
	case ClientMessage:
		if(event->xclient.data.l[0] != ATOM_WM_DELETE_WINDOW) {
			return;
		}
		signal_id = my_win_signals[MY_WIN_SIGNAL_DELETE_EVENT];
		break;
	case ConfigureNotify:
		signal_id = my_win_signals[MY_WIN_SIGNAL_CONFIGURE_NOTIFY];
		break;
	default:
		return;
	}

	g_signal_emit(self, signal_id, 0, event);
}

gboolean x_event_prepare(GSource * self, gint * timeout)
{
	* timeout = -1;
	return FALSE;
}

gboolean x_event_check(GSource * self)
{
	return XPending(display) > 0;
}

gboolean x_event_dispatch(GSource * source, GSourceFunc callback, gpointer user_data)
{
	XEvent event;
	XNextEvent(display, & event);

	g_message("x_event_dispatch: %d", event.type);

	MyWin * win = g_hash_table_lookup(windows, GUINT_TO_POINTER(event.xany.window));
	if(! win) {
		g_warning("Unkown window %lu", event.xany.window);
		return TRUE;
	}
	
	my_win_dispatch_event(win, & event);
	
	return TRUE;
}

void on_window_delete()
{
	if(g_hash_table_size(windows) > 0) {
		return;
	}
	
	g_message("Bye");
	g_main_loop_quit(loop);
}

void on_window_configured(MyWin * my_win)
{
	g_message("New dimension: %d %d %d %d",
		my_win->x, my_win->y,
		my_win->width, my_win->height);
}

int main()
{
	g_type_init();
	
	// table for mapping Window to MyWin
	windows = g_hash_table_new(g_direct_hash, g_direct_equal);

	display = XOpenDisplay(NULL);
	if(NULL == display) {
		g_error("Failed to initialize display");
	}
	ATOM_WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", FALSE);
	
	GSourceFuncs funcs = {
		x_event_prepare,
		x_event_check,
		x_event_dispatch,
		NULL
	};
	GPollFD poll_fd;
	poll_fd.fd = XConnectionNumber(display);
	poll_fd.events = G_IO_IN;
	GSource * source = g_source_new(& funcs, sizeof(GSource));
	g_source_add_poll(source, & poll_fd);
	g_source_set_can_recurse(source, TRUE);
	g_source_attach(source, NULL);
	g_source_unref(source);
	g_message("Listen on fd %d", poll_fd.fd);

	MyWin * my_win = g_object_new(MY_TYPE_WIN, NULL);
	my_win_realize(my_win);
	g_signal_connect(my_win, "delete-event", G_CALLBACK(on_window_delete), NULL);
	g_signal_connect(my_win, "configure-notify", G_CALLBACK(on_window_configured), NULL);
	
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	g_object_unref(my_win);
	XCloseDisplay(display);
	
	return 0;
}
