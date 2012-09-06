#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-screen-private.h"

#define XDK_DISPLAY_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE(o, XDK_TYPE_DISPLAY, XdkDisplayPrivate))

#define XDK_DISPLAY_SOURCE(o) ((XdkDisplaySource *) (o))

#define XDK_EVENT_FILTER_NODE(o) ((XdkEventFilterNode *) (o))

typedef struct _XdkDisplaySource XdkDisplaySource;

typedef struct _XdkEventFilterNode XdkEventFilterNode;

struct _XdkDisplayPrivate
{
	Display * peer;
	
	gchar * name;
	
	gint n_screens;
	
	gint default_screen;
	
	XdkScreen ** screens;
	
	GHashTable * windows;
	
	GPollFD poll_fd;
	
	guint event_watch_id;
	
	GList * event_filters;
};

struct _XdkDisplaySource
{
	GSource base;
	
	XdkDisplay * display;
};

struct _XdkEventFilterNode
{
	XdkEventFilter filter;
	
	gpointer user_data;
};

enum {
	SIGNAL_DISCONNECT,
	SIGNAL_MAX,
};

static void xdk_display_dispose(GObject * object);

static void xdk_display_finalize(GObject * object);

static int xdk_display_on_error(Display * display, XErrorEvent * error);
	
static XdkEventFilterNode * xdk_event_filter_node_dup(XdkEventFilterNode * self);

static void xdk_event_filter_node_free(XdkEventFilterNode * self);

static gint xdk_event_filter_node_compare(gconstpointer a, gconstpointer b);

static gboolean xdk_display_dump_event(Display * display, XEvent * event);

GQuark XDK_ATOM_WM_DELETE_WINDOW = 0;

G_DEFINE_TYPE(XdkDisplay, xdk_display, G_TYPE_OBJECT);

static guint signals[SIGNAL_MAX] = { 0, };

static XdkDisplay * xdk_default_display = NULL;

static GError * xdk_last_error = NULL;

void xdk_display_class_init(XdkDisplayClass * clazz)
{
	GObjectClass * gobject_clazz = G_OBJECT_CLASS(clazz);
	gobject_clazz->dispose = xdk_display_dispose;
	gobject_clazz->finalize = xdk_display_finalize;
	
	GType type = G_TYPE_FROM_CLASS(clazz);
	signals[SIGNAL_DISCONNECT] = g_signal_new(
		"disconnect",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		g_cclosure_marshal_VOID__POINTER,
		G_TYPE_NONE, 0);
		
	g_type_class_add_private(clazz, sizeof(XdkDisplayPrivate));
	
	XDK_ATOM_WM_DELETE_WINDOW = g_quark_from_static_string("WM_DELETE_WINDOW");
}

static void xdk_display_init(XdkDisplay * self)
{
	XdkDisplayPrivate * priv = XDK_DISPLAY_GET_PRIVATE(self);
	self->priv = priv;

	gboolean result = FALSE;
	priv->peer = XOpenDisplay(g_getenv("DISPLAY"));
	if(! priv->peer) {
		g_error("Failed to initialize display");
	}
	
	priv->name = XDisplayString(priv->peer);
	priv->default_screen = XDefaultScreen(priv->peer);
	priv->n_screens = XScreenCount(priv->peer);
	priv->screens = g_malloc0(sizeof(XdkScreen *) * priv->n_screens);
	priv->windows = g_hash_table_new(g_direct_hash, g_direct_equal);
	
	// display ready to expose to outside world
	xdk_default_display = self;

	int i;
	for(i = 0; i < priv->n_screens; i ++) {
		XdkScreen * screen = xdk_screen_new();
		if(! screen) {
			return;
		}
		xdk_screen_set_display(screen, self);
		xdk_screen_set_peer(screen, XScreenOfDisplay(priv->peer, i));
		priv->screens[i] = screen;
	}
	
	for(i = 0; i < priv->n_screens; i ++) {
		xdk_display_add_window(
			self,
			xdk_screen_get_root_window(priv->screens[i]));
	}
	
	Atom atom = XInternAtom(priv->peer, "WM_DELETE_WINDOW", FALSE);
	g_object_set_qdata(
		G_OBJECT(self),
		XDK_ATOM_WM_DELETE_WINDOW,
		GUINT_TO_POINTER(atom));
		
	if(g_getenv("XDK_DUMP_EVENT")) {
		xdk_display_add_event_filter(
			self,
			(XdkEventFilter) xdk_display_dump_event,
			NULL);
	}
}

static void xdk_display_dispose(GObject * object)
{
	XdkDisplayPrivate * priv = XDK_DISPLAY(object)->priv;
	
	int i = 0;
	for(; i < priv->n_screens; i ++) {
		if(! priv->screens[i]) {
			continue;
		}
		
		g_object_unref(priv->screens[i]);
	}
}

static void xdk_display_finalize(GObject * object)
{
	XdkDisplayPrivate * priv = XDK_DISPLAY(object)->priv;

	g_free(priv->screens);
	
	if(priv->peer) {
		XCloseDisplay(priv->peer);
	}
	
	G_OBJECT_CLASS(xdk_display_parent_class)->finalize(object);
}

XdkDisplay * xdk_display_get_default()
{
	if(! xdk_default_display) {
		g_object_new(XDK_TYPE_DISPLAY, NULL);
	}
	
	return xdk_default_display;
}

Display * xdk_display_get_peer(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->peer;
}

const gchar * xdk_display_get_name(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->name;
}

gint xdk_display_get_n_screens(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->n_screens;
}

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	XdkDisplayPrivate * priv = self->priv;
	g_return_val_if_fail(priv->screens, NULL);
	
	return priv->screens[priv->default_screen];
}

void xdk_display_flush(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XFlush(self->priv->peer);
}

void xdk_flush()
{
	xdk_display_flush(xdk_display_get_default());
}

const char * xdk_display_get_vendor(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return XServerVendor(self->priv->peer);
}

gint xdk_display_get_release(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return XVendorRelease(self->priv->peer);
}

XdkScreen * xdk_get_default_screen()
{
	return xdk_display_get_default_screen(xdk_display_get_default());
}

XdkWindow * xdk_get_default_root_window()
{
	return xdk_screen_get_root_window(xdk_get_default_screen());
}

Atom xdk_display_atom_from_name(
	XdkDisplay * self,
	const char * atom_name,
	gboolean only_if_exists)
{
	g_return_val_if_fail(self, None);
	
	return XInternAtom(self->priv->peer, atom_name, only_if_exists);
}
	
gchar * xdk_display_atom_to_name(XdkDisplay * self, Atom atom)
{
	g_return_val_if_fail(self, NULL);
	
	return XGetAtomName(self->priv->peer, atom);;
}

Atom xdk_display_atom_get(XdkDisplay * self, GQuark name)
{
	g_return_val_if_fail(self, None);
	g_return_val_if_fail(name, None);
	
	Atom atom = (Atom) g_object_get_qdata(G_OBJECT(self), name);
	if(None == atom) {
		atom = xdk_display_atom_from_name(
			self,
			g_quark_to_string(name),
			FALSE);
		g_object_set_qdata(G_OBJECT(self), name, GUINT_TO_POINTER(atom));
	}
	
	return atom;
}

Atom xdk_atom_from_name(
	const char * atom_name,
	gboolean only_if_exists)
{
	return xdk_display_atom_from_name(
		xdk_display_get_default(),
		atom_name,
		only_if_exists);
}

gchar * xdk_atom_to_name(Atom atom)
{
	return xdk_display_atom_to_name(xdk_display_get_default(), atom);
}

static xdk_display_window_destroyed(
	XdkWindow * window,
	XdkDisplay * self)
{
	xdk_display_remove_window(self, window);
}

void xdk_display_add_window(XdkDisplay * self, XdkWindow * window)
{
	g_return_if_fail(self && window);
	Window xwin = xdk_window_get_peer(window);
	g_return_if_fail(None != xwin);
	
	g_signal_connect(
		window, "destroy",
		G_CALLBACK(xdk_display_window_destroyed), self);
	
	g_hash_table_insert(
		self->priv->windows,
		GUINT_TO_POINTER(xwin),
		window);
}

XdkWindow * xdk_display_lookup_window(XdkDisplay * self, Window xwindow)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(None != xwindow, NULL);
	
	return g_hash_table_lookup(self->priv->windows, GUINT_TO_POINTER(xwindow));
}

gboolean xdk_display_has_window(XdkDisplay * self, Window xwindow)
{
	g_return_val_if_fail(self, FALSE);
	
	return g_hash_table_contains(self->priv->windows, GUINT_TO_POINTER(xwindow));
}

void xdk_display_remove_window(XdkDisplay * self, XdkWindow * window)
{
	g_return_if_fail(self && window);
	Window xwin = xdk_window_get_peer(window);
	g_return_if_fail(None != xwin);
	
	g_hash_table_remove(self->priv->windows, GUINT_TO_POINTER(xwin));
}

int xdk_display_get_connection_number(XdkDisplay * self)
{
	g_return_val_if_fail(self, -1);
	
	return XConnectionNumber(self->priv->peer);
}

static void xdk_display_dispatch_event(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XEvent event;
	XdkDisplayPrivate * priv = self->priv;
	XNextEvent(priv->peer, & event);
	GList * node = g_list_last(priv->event_filters);
	while(node) {
		XdkEventFilterNode * filter_node = XDK_EVENT_FILTER_NODE(node->data);
		if(! filter_node->filter(self, & event, filter_node->user_data)) {
			return;
		}
		node = g_list_previous(node);
	}
	
	XdkWindow * window = xdk_display_lookup_window(
		self,
		event.xany.window);
	if(! window) {
		return;
	}
	
	xdk_window_dispatch_event(window, & event);
}

static gboolean xdk_display_source_prepare(GSource * source, gint * timeout)
{
	* timeout = -1;
	
	XdkDisplayPrivate * priv = XDK_DISPLAY_SOURCE(source)->display->priv;
	
	return XPending(priv->peer);
}

static gboolean xdk_display_source_check(GSource * source)
{
	XdkDisplayPrivate * priv = XDK_DISPLAY_SOURCE(source)->display->priv;
	if(priv->poll_fd.revents & (G_IO_ERR | G_IO_HUP)) {
		g_warning("Disconnected from X server");
		
		g_source_remove(priv->event_watch_id);
		priv->event_watch_id = 0;
	
		g_signal_emit(
			XDK_DISPLAY_SOURCE(source)->display,
			signals[SIGNAL_DISCONNECT],
			0);
		
		return FALSE;
	}
	
	int num_events = XPending(priv->peer);
	
	g_debug("xdk_display_source_check: %d event pendding", num_events);
	
	return num_events;
}

static gboolean xdk_display_source_dispatch(
	GSource * source,
	GSourceFunc callback,
	gpointer user_data)
{
	callback(user_data);
	
	return TRUE;
}

GSourceFuncs xdk_display_source_funcs = {
	xdk_display_source_prepare,
	xdk_display_source_check,
	xdk_display_source_dispatch,
	NULL
};

void xdk_display_add_watch(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XdkDisplayPrivate * priv = self->priv;
	if(priv->event_watch_id) {
		return;
	}
	
	GSource * source = xdk_display_watch_source_new(self);
	g_source_set_name(source, "XdkDisplayEventSource");
	g_source_set_can_recurse(source, TRUE);
	priv->event_watch_id = g_source_attach(source, NULL);
	g_source_unref(source);
}

void xdk_display_remove_watch(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XdkDisplayPrivate * priv = self->priv;
	if(! priv->event_watch_id) {
		return;
	}
	
	g_source_remove(priv->event_watch_id);
}

GSource * xdk_display_watch_source_new(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	XdkDisplayPrivate * priv = self->priv;
	priv->poll_fd.fd = xdk_display_get_connection_number(self);
	priv->poll_fd.events = G_IO_IN | G_IO_ERR | G_IO_HUP;
	
	g_debug("xdk_display_watch_source_new: fd = %d", priv->poll_fd.fd);
	
	GSource * source = g_source_new(
		& xdk_display_source_funcs,
		sizeof(XdkDisplaySource));
	g_source_add_poll(source, & priv->poll_fd);
	g_source_set_callback(
		source,
		(GSourceFunc) xdk_display_dispatch_event, self,
		NULL);
	XDK_DISPLAY_SOURCE(source)->display = self;
		
	return source;
}

void xdk_display_add_event_filter(
	XdkDisplay * self,
	XdkEventFilter filter,
	gpointer user_data)
{
	g_return_if_fail(filter);
	
	if(NULL == self) {
		self = xdk_display_get_default();
	}
	
	XdkDisplayPrivate * priv = self->priv;
	XdkEventFilterNode node = {
		.filter = filter,
		.user_data = user_data
	};
	if(g_list_find_custom(priv->event_filters, & node, xdk_event_filter_node_compare)) {
		return;
	}

	priv->event_filters = g_list_prepend(priv->event_filters, xdk_event_filter_node_dup(& node));
}

void xdk_display_remove_event_filter(
	XdkDisplay * self,
	XdkEventFilter filter,
	gpointer user_data)
{
	g_return_if_fail(filter);
	
	if(NULL == self) {
		self = xdk_display_get_default();
	}
	
	XdkDisplayPrivate * priv = self->priv;
	XdkEventFilterNode filter_node = {
		.filter = filter,
		.user_data = user_data
	};
	GList * node = g_list_find_custom(
		priv->event_filters,
		& filter_node,
		xdk_event_filter_node_compare);
	if(! node) {
		return;
	}
	
	xdk_event_filter_node_free(node->data);
	priv->event_filters = g_list_delete_link(priv->event_filters, node);
}

GList * xdk_display_list_screen(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	int i = 0;
	GList * screens = NULL;
	for(; i < xdk_display_get_n_screens(self); i ++) {
		screens = g_list_append(screens, self->priv->screens[i]);
	}
	
	return screens;
}

XdkScreen * xdk_display_lookup_screen(XdkDisplay * self, Screen * screen)
{
	g_return_val_if_fail(self, NULL);
	
	int i = 0;
	XdkScreen * xdk_screen = NULL;
	for(; i < xdk_display_get_n_screens(self); i ++) {
		if(xdk_screen_get_peer(self->priv->screens[i]) == screen) {
			xdk_screen = self->priv->screens[i];
			break;
		}
	}
	
	return xdk_screen;
}

gint xdk_display_get_n_windows(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return g_hash_table_size(self->priv->windows);
}

GList * xdk_display_list_windows(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return g_hash_table_get_values(self->priv->windows);
}

GList * xdk_display_list_xwindows(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return g_hash_table_get_keys(self->priv->windows);
}

static XdkEventFilterNode * xdk_event_filter_node_dup(XdkEventFilterNode * self)
{
	g_return_val_if_fail(self, NULL);
	
	return g_slice_dup(XdkEventFilterNode, self);
}

static void xdk_event_filter_node_free(XdkEventFilterNode * self)
{
	g_return_if_fail(self);
	
	g_slice_free(XdkEventFilterNode, self);
}

static gint xdk_event_filter_node_compare(gconstpointer a, gconstpointer b)
{
	gint result = XDK_EVENT_FILTER_NODE(a)->filter - XDK_EVENT_FILTER_NODE(b)->filter;
	if(result) {
		return result;
	}
	
	return XDK_EVENT_FILTER_NODE(a)->user_data - XDK_EVENT_FILTER_NODE(b)->user_data;
}

static gboolean xdk_display_dump_event(Display * display, XEvent * event)
{
	xdk_util_event_dump(event);
	
	return TRUE;
}

static int xdk_display_on_error(Display * display, XErrorEvent * error)
{
	g_return_val_if_fail(! xdk_last_error, 0);

	xdk_last_error = xdk_error_new(error);
	
	return 1;
}

void xdk_trap_error()
{
	XSetErrorHandler(xdk_display_on_error);
}

gint xdk_untrap_error(GError ** error)
{
	XdkDisplay * display = xdk_display_get_default();
	XSync(xdk_display_get_peer(display), FALSE);
	XSetErrorHandler(NULL);
	
	gint error_code = XDK_ERROR_SUCCESS;
	if(xdk_last_error) {
		error_code = xdk_last_error->code;
		g_propagate_error(error, xdk_last_error);
		xdk_last_error = NULL;
	}
	
	return error_code;
}
