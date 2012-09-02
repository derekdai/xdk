#include <glib-object.h>
#include <X11/Xlib.h>

#define MY_TYPE_WIN (my_win_get_type())
#define MY_WIN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MY_TYPE_WIN, MyWin))

typedef struct _MyWinClass MyWinClass;

typedef struct _MyWin MyWin;

struct _MyWinClass
{
	GObjectClass base;
	
	void (* delete_event)(MyWin * self);
};

struct _MyWin
{
	GObject base;
	
	Window peer;
};

void my_win_handle_delete_event(MyWin * self);

void my_win_finalize(GObject * object);

G_DEFINE_TYPE(MyWin, my_win, G_TYPE_OBJECT);

Display * display;

GHashTable * windows;

guint delete_event_signal;

GMainLoop * loop;

Atom ATOM_WM_DELETE_WINDOW;

void my_win_class_init(MyWinClass * clazz)
{
	GObjectClass * object_class = G_OBJECT_CLASS(clazz);
	object_class->finalize = my_win_finalize;
	
	clazz->delete_event = my_win_handle_delete_event;
	
	delete_event_signal = g_signal_new(
		"delete-event",
		G_TYPE_FROM_CLASS(clazz),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(MyWinClass, delete_event),
		NULL, NULL,
		g_cclosure_marshal_generic,
		G_TYPE_NONE, 0);
}

void my_win_init(MyWin * self)
{
	Window root = DefaultRootWindow(display);
	if(None == root) {
		g_error("No root window found");
	}

	self->peer = XCreateSimpleWindow(
		display, root,
		0, 0, 1280, 720,
		0, 0,
		0xffffffff);
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

void my_win_handle_delete_event(MyWin * self)
{
	// destroy if we receive delete event sent by window manager
	my_win_destroy(self);
}

gboolean x_event_prepare(GSource * self, gint * timeout)
{
	g_message("x_event_prepare");
	
	* timeout = -1;
	return FALSE;
}

gboolean x_event_check(GSource * self)
{
	g_message("x_event_check: %d", XPending(display));
	
	return XPending(display) > 0;
}

gboolean x_event_dispatch(GSource * source, GSourceFunc callback, gpointer user_data)
{
	XEvent event;
	XNextEvent(display, & event);

	g_message("x_event_dispatch: %d", event.type);

	if(ClientMessage != event.type) {
		return FALSE;
	}

	if(event.xclient.data.l[0] != ATOM_WM_DELETE_WINDOW) {
		return FALSE;
	}

	MyWin * win = g_hash_table_lookup(windows, GUINT_TO_POINTER(event.xany.window));
	if(! win) {
		g_warning("Unkown window %lu", event.xany.window);
		return FALSE;
	}
	
	g_message("WM_DELETE_WINDOW dispatched to window %lu", event.xany.window);
	g_signal_emit(win, delete_event_signal, 0);
}

void on_window_delete()
{
	if(g_hash_table_size(windows) > 0) {
		return;
	}
	
	g_message("Bye");
	g_main_loop_quit(loop);
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
	g_signal_connect(my_win, "delete-event", G_CALLBACK(on_window_delete), NULL);
	
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	g_object_unref(my_win);
	XCloseDisplay(display);
	
	return 0;
}
