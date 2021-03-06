#include "xdk-display.h"
#include "xdk-screen.h"
#include <X11/Xlibint.h>

#define XDK_DISPLAY_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE(o, XDK_TYPE_DISPLAY, XdkDisplayPrivate))

#define XDK_DISPLAY_SOURCE(o) ((XdkDisplaySource *) (o))

#define XDK_EVENT_FILTER_NODE(o) ((XdkEventFilterNode *) (o))

typedef struct _XdkDisplaySource XdkDisplaySource;

typedef struct _XdkEventFilterNode XdkEventFilterNode;

struct _XdkDisplayPrivate
{
	Display * peer;
	
	char * display_string;
	
	gint n_screens;
	
	GPtrArray * screens;
	
	GHashTable * windows;
	
	GPollFD poll_fd;
	
	guint event_watch_id;
	
	GList * event_filters;
	
	gboolean own_peer : 1;
	
	gboolean event_retrieval_disabled : 1;
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

enum {
	PROP_PEER = 1,
	PROP_DISPLAY_STRING,
	PROP_EVENT_RETRIEVAL_DISABLED
};

static void xdk_display_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec);

static void xdk_display_dispose(GObject * object);

static void xdk_display_finalize(GObject * object);

static void xdk_display_set_property(
	GObject        *object,
	guint           property_id,
	const GValue   *value,
	GParamSpec     *pspec);

static XdkEventFilterNode * xdk_event_filter_node_dup(XdkEventFilterNode * self);

static void xdk_event_filter_node_free(XdkEventFilterNode * self);

static gint xdk_event_filter_node_compare(gconstpointer a, gconstpointer b);

static gboolean xdk_display_dump_event(Display * display, XEvent * event);

static int xdk_display_default_error_handler(Display * display, XErrorEvent * error);

G_DEFINE_TYPE(XdkDisplay, xdk_display, G_TYPE_OBJECT);

static guint signals[SIGNAL_MAX] = { 0, };

volatile XdkDisplay * xdk_default_display = NULL;

static GError * xdk_last_error = NULL;

static gboolean xdk_dump_error = FALSE;

GQuark XDK_ATOM_WM_DELETE_WINDOW;
GQuark XDK_ATOM_NET_SUPPORTED;
GQuark XDK_ATOM_NET_CLIENT_LIST;
GQuark XDK_ATOM_NET_NUMBER_OF_DESKTOPS;
GQuark XDK_ATOM_NET_DESKTOP_GEOMETRY;
GQuark XDK_ATOM_NET_DESKTOP_VIEWPORT;
GQuark XDK_ATOM_NET_CURRENT_DESKTOP;
GQuark XDK_ATOM_NET_DESKTOP_NAMES;
GQuark XDK_ATOM_NET_ACTIVE_WINDOW;
GQuark XDK_ATOM_NET_WORKAREA;
GQuark XDK_ATOM_NET_SUPPORTING_WM_CHECK;
GQuark XDK_ATOM_NET_VIRTUAL_ROOTS;
GQuark XDK_ATOM_NET_DESKTOP_LAYOUT;
GQuark XDK_ATOM_NET_SHOWING_DESKTOP;
GQuark XDK_ATOM_NET_WINDOW;
GQuark XDK_ATOM_NET_MOVERESIZE_WINDOW;
GQuark XDK_ATOM_NET_WM_MOVERESIZE;
GQuark XDK_ATOM_NET_RESTACK_WINDOW;
GQuark XDK_ATOM_NET_REQUEST_FRAME_EXTENTS;
GQuark XDK_ATOM_NET_WM_NAME;
GQuark XDK_ATOM_NET_WM_VISIBLE_NAME;
GQuark XDK_ATOM_NET_WM_ICON_NAME;
GQuark XDK_ATOM_NET_WM_VISIBLE_ICON_NAME;
GQuark XDK_ATOM_NET_WM_DESKTOP;
GQuark XDK_ATOM_NET_WM_WINDOW_TYPE;
GQuark XDK_ATOM_NET_WM_STATE;
GQuark XDK_ATOM_NET_WM_ALLOWED_ACTIONS;
GQuark XDK_ATOM_NET_WM_STRUT;
GQuark XDK_ATOM_NET_WM_STRUT_PARTIAL;
GQuark XDK_ATOM_NET_WM_ICON_GEOMETRY;
GQuark XDK_ATOM_NET_WM_ICON;
GQuark XDK_ATOM_NET_WM_PID;
GQuark XDK_ATOM_NET_WM_HANDLED_ICONS;
GQuark XDK_ATOM_NET_WM_USER_TIME;
GQuark XDK_ATOM_NET_WM_USER_TIME_WINDOW;
GQuark XDK_ATOM_NET_FRAME_EXTENTS;
GQuark XDK_ATOM_NET_WM_OPAQUE_REGION;
GQuark XDK_ATOM_NET_WM_PING;
GQuark XDK_ATOM_NET_WM_SYNC_REQUEST;
GQuark XDK_ATOM_NET_WM_FULLSCREEN_MONITORS;
GQuark XDK_ATOM_NET_WM_FULLPLACEMENT;
GQuark XDK_ATOM_NET_WM_CM_S0;
GQuark XDK_ATOM_NET_WM_CM_S1;
GQuark XDK_ATOM_NET_WM_CM_S2;
GQuark XDK_ATOM_NET_WM_CM_S3;
GQuark XDK_ATOM_WM_TRANSIENT_FOR;
GQuark XDK_ATOM_WM_NAME;
GQuark XDK_ATOM_WM_ICON_NAME;
GQuark XDK_ATOM_WM_NORMAL_HINTS;
GQuark XDK_ATOM_WM_HINTS;
GQuark XDK_ATOM_WM_CLASS;
GQuark XDK_ATOM_WM_TRANSIENT_FOR;
GQuark XDK_ATOM_WM_PROTOCOLS;
GQuark XDK_ATOM_WM_COLORMAP_WINDOWS;
GQuark XDK_ATOM_WM_CLIENT_MACHINE;
GQuark XDK_ATOM_WM_STATE;
GQuark XDK_ATOM_WM_ICON_SIZE;
GQuark XDK_ATOM_NET_WM_STATE_MODAL;
GQuark XDK_ATOM_NET_WM_STATE_STICKY;
GQuark XDK_ATOM_NET_WM_STATE_MAXIMIZED_VERT;
GQuark XDK_ATOM_NET_WM_STATE_MAXIMIZED_HORZ;
GQuark XDK_ATOM_NET_WM_STATE_SHADED;
GQuark XDK_ATOM_NET_WM_STATE_SKIP_TASKBAR;
GQuark XDK_ATOM_NET_WM_STATE_SKIP_PAGER;
GQuark XDK_ATOM_NET_WM_STATE_HIDDEN;
GQuark XDK_ATOM_NET_WM_STATE_FULLSCREEN;
GQuark XDK_ATOM_NET_WM_STATE_ABOVE;
GQuark XDK_ATOM_NET_WM_STATE_BELOW;
GQuark XDK_ATOM_NET_WM_STATE_DEMANDS_ATTENTION;
GQuark XDK_ATOM_NET_WM_STATE_FOCUSED;

void xdk_display_class_init(XdkDisplayClass * clazz)
{
	GObjectClass * gobject_clazz = G_OBJECT_CLASS(clazz);
	gobject_clazz->set_property = xdk_display_set_property;
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
		
	g_object_class_install_property(
		gobject_clazz,
		PROP_PEER,
		g_param_spec_pointer(
			"peer", "", "",
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	
	g_object_class_install_property(
		gobject_clazz,
		PROP_DISPLAY_STRING,
		g_param_spec_string(
			"display-string", "", "",
			NULL,
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	
	g_object_class_install_property(
		gobject_clazz,
		PROP_EVENT_RETRIEVAL_DISABLED,
		g_param_spec_boolean(
			"event-retreival-disabled", "", "",
			FALSE,
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	
	XDK_ATOM_WM_DELETE_WINDOW = g_quark_from_static_string("WM_DELETE_WINDOW");
	XDK_ATOM_WM_DELETE_WINDOW = g_quark_from_static_string("WM_DELETE_WINDOW");
	XDK_ATOM_NET_SUPPORTED = g_quark_from_static_string("_NET_SUPPORTED");
	XDK_ATOM_NET_CLIENT_LIST = g_quark_from_static_string("_NET_CLIENT_LIST");
	XDK_ATOM_NET_NUMBER_OF_DESKTOPS = g_quark_from_static_string("_NET_NUMBER_OF_DESKTOPS");
	XDK_ATOM_NET_DESKTOP_GEOMETRY = g_quark_from_static_string("_NET_DESKTOP_GEOMETRY");
	XDK_ATOM_NET_DESKTOP_VIEWPORT = g_quark_from_static_string("_NET_DESKTOP_VIEWPORT");
	XDK_ATOM_NET_CURRENT_DESKTOP = g_quark_from_static_string("_NET_CURRENT_DESKTOP");
	XDK_ATOM_NET_DESKTOP_NAMES = g_quark_from_static_string("_NET_DESKTOP_NAMES");
	XDK_ATOM_NET_ACTIVE_WINDOW = g_quark_from_static_string("_NET_ACTIVE_WINDOW");
	XDK_ATOM_NET_WORKAREA = g_quark_from_static_string("_NET_WORKAREA");
	XDK_ATOM_NET_SUPPORTING_WM_CHECK = g_quark_from_static_string("_NET_SUPPORTING_WM_CHECK");
	XDK_ATOM_NET_VIRTUAL_ROOTS = g_quark_from_static_string("_NET_VIRTUAL_ROOTS");
	XDK_ATOM_NET_DESKTOP_LAYOUT = g_quark_from_static_string("_NET_DESKTOP_LAYOUT");
	XDK_ATOM_NET_SHOWING_DESKTOP = g_quark_from_static_string("_NET_SHOWING_DESKTOP");
	XDK_ATOM_NET_WINDOW = g_quark_from_static_string("_NET_WINDOW");
	XDK_ATOM_NET_MOVERESIZE_WINDOW = g_quark_from_static_string("_NET_MOVERESIZE_WINDOW");
	XDK_ATOM_NET_WM_MOVERESIZE = g_quark_from_static_string("_NET_WM_MOVERESIZE");
	XDK_ATOM_NET_RESTACK_WINDOW = g_quark_from_static_string("_NET_RESTACK_WINDOW");
	XDK_ATOM_NET_REQUEST_FRAME_EXTENTS = g_quark_from_static_string("_NET_REQUEST_FRAME_EXTENTS");
	XDK_ATOM_NET_WM_NAME = g_quark_from_static_string("_NET_WM_NAME");
	XDK_ATOM_NET_WM_VISIBLE_NAME = g_quark_from_static_string("_NET_WM_VISIBLE_NAME");
	XDK_ATOM_NET_WM_ICON_NAME = g_quark_from_static_string("_NET_WM_ICON_NAME");
	XDK_ATOM_NET_WM_VISIBLE_ICON_NAME = g_quark_from_static_string("_NET_WM_VISIBLE_ICON_NAME");
	XDK_ATOM_NET_WM_DESKTOP = g_quark_from_static_string("_NET_WM_DESKTOP");
	XDK_ATOM_NET_WM_WINDOW_TYPE = g_quark_from_static_string("_NET_WM_WINDOW_TYPE");
	XDK_ATOM_NET_WM_STATE = g_quark_from_static_string("_NET_WM_STATE");
	XDK_ATOM_NET_WM_ALLOWED_ACTIONS = g_quark_from_static_string("_NET_WM_ALLOWED_ACTIONS");
	XDK_ATOM_NET_WM_STRUT = g_quark_from_static_string("_NET_WM_STRUT");
	XDK_ATOM_NET_WM_STRUT_PARTIAL = g_quark_from_static_string("_NET_WM_STRUT_PARTIAL");
	XDK_ATOM_NET_WM_ICON_GEOMETRY = g_quark_from_static_string("_NET_WM_ICON_GEOMETRY");
	XDK_ATOM_NET_WM_ICON = g_quark_from_static_string("_NET_WM_ICON");
	XDK_ATOM_NET_WM_PID = g_quark_from_static_string("_NET_WM_PID");
	XDK_ATOM_NET_WM_HANDLED_ICONS = g_quark_from_static_string("_NET_WM_HANDLED_ICONS");
	XDK_ATOM_NET_WM_USER_TIME = g_quark_from_static_string("_NET_WM_USER_TIME");
	XDK_ATOM_NET_WM_USER_TIME_WINDOW = g_quark_from_static_string("_NET_WM_USER_TIME_WINDOW");
	XDK_ATOM_NET_FRAME_EXTENTS = g_quark_from_static_string("_NET_FRAME_EXTENTS");
	XDK_ATOM_NET_WM_OPAQUE_REGION = g_quark_from_static_string("_NET_WM_OPAQUE_REGION");
	XDK_ATOM_NET_WM_PING = g_quark_from_static_string("_NET_WM_PING");
	XDK_ATOM_NET_WM_SYNC_REQUEST = g_quark_from_static_string("_NET_WM_SYNC_REQUEST");
	XDK_ATOM_NET_WM_FULLSCREEN_MONITORS = g_quark_from_static_string("_NET_WM_FULLSCREEN_MONITORS");
	XDK_ATOM_NET_WM_FULLPLACEMENT = g_quark_from_static_string("_NET_WM_FULLPLACEMENT");
	XDK_ATOM_NET_WM_CM_S0 = g_quark_from_static_string("_NET_WM_CM_S0");
	XDK_ATOM_NET_WM_CM_S1 = g_quark_from_static_string("_NET_WM_CM_S1");
	XDK_ATOM_NET_WM_CM_S2 = g_quark_from_static_string("_NET_WM_CM_S2");
	XDK_ATOM_NET_WM_CM_S3 = g_quark_from_static_string("_NET_WM_CM_S3");
	XDK_ATOM_WM_TRANSIENT_FOR = g_quark_from_static_string("XDK_ATOM_WM_TRANSIENT_FOR");
	XDK_ATOM_WM_NAME = g_quark_from_static_string("XDK_ATOM_WM_NAME");
	XDK_ATOM_WM_ICON_NAME = g_quark_from_static_string("XDK_ATOM_WM_ICON_NAME");
	XDK_ATOM_WM_NORMAL_HINTS = g_quark_from_static_string("XDK_ATOM_WM_NORMAL_HINTS");
	XDK_ATOM_WM_HINTS = g_quark_from_static_string("XDK_ATOM_WM_HINTS");
	XDK_ATOM_WM_CLASS = g_quark_from_static_string("XDK_ATOM_WM_CLASS");
	XDK_ATOM_WM_TRANSIENT_FOR = g_quark_from_static_string("XDK_ATOM_WM_TRANSIENT_FOR");
	XDK_ATOM_WM_PROTOCOLS = g_quark_from_static_string("XDK_ATOM_WM_PROTOCOLS");
	XDK_ATOM_WM_COLORMAP_WINDOWS = g_quark_from_static_string("XDK_ATOM_WM_COLORMAP_WINDOWS");
	XDK_ATOM_WM_CLIENT_MACHINE = g_quark_from_static_string("XDK_ATOM_WM_CLIENT_MACHINE");
	XDK_ATOM_WM_STATE = g_quark_from_static_string("XDK_ATOM_WM_STATE");
	XDK_ATOM_WM_ICON_SIZE = g_quark_from_static_string("XDK_ATOM_WM_ICON_SIZE");
	XDK_ATOM_NET_WM_STATE_MODAL = g_quark_from_static_string("_NET_WM_STATE_MODAL");
	XDK_ATOM_NET_WM_STATE_STICKY = g_quark_from_static_string("_NET_WM_STATE_STICKY");
	XDK_ATOM_NET_WM_STATE_MAXIMIZED_VERT = g_quark_from_static_string("_NET_WM_STATE_MAXIMIZED_VERT");
	XDK_ATOM_NET_WM_STATE_MAXIMIZED_HORZ = g_quark_from_static_string("_NET_WM_STATE_MAXIMIZED_HORZ");
	XDK_ATOM_NET_WM_STATE_SHADED = g_quark_from_static_string("_NET_WM_STATE_SHADED");
	XDK_ATOM_NET_WM_STATE_SKIP_TASKBAR = g_quark_from_static_string("_NET_WM_STATE_SKIP_TASKBAR");
	XDK_ATOM_NET_WM_STATE_SKIP_PAGER = g_quark_from_static_string("_NET_WM_STATE_SKIP_PAGER");
	XDK_ATOM_NET_WM_STATE_HIDDEN = g_quark_from_static_string("_NET_WM_STATE_HIDDEN");
	XDK_ATOM_NET_WM_STATE_FULLSCREEN = g_quark_from_static_string("_NET_WM_STATE_FULLSCREEN");
	XDK_ATOM_NET_WM_STATE_ABOVE = g_quark_from_static_string("_NET_WM_STATE_ABOVE");
	XDK_ATOM_NET_WM_STATE_BELOW = g_quark_from_static_string("_NET_WM_STATE_BELOW");
	XDK_ATOM_NET_WM_STATE_DEMANDS_ATTENTION = g_quark_from_static_string("_NET_WM_STATE_DEMANDS_ATTENTION");
	XDK_ATOM_NET_WM_STATE_FOCUSED = g_quark_from_static_string("_NET_WM_STATE_FOCUSED");

	if(g_getenv("XDK_DUMP_ERROR")) {
		xdk_dump_error = TRUE;
		XSetErrorHandler(xdk_display_default_error_handler);
	}
	
	g_type_class_add_private(clazz, sizeof(XdkDisplayPrivate));
}

static void xdk_display_init(XdkDisplay * self)
{
	XdkDisplayPrivate * priv = XDK_DISPLAY_GET_PRIVATE(self);
	self->priv = priv;

	priv->windows = g_hash_table_new_full(
		g_direct_hash, g_direct_equal,
		NULL, (GDestroyNotify) g_object_unref);
		
	priv->screens = g_ptr_array_new_with_free_func(g_object_unref);
}

static void xdk_display_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec)
{
	XdkDisplayPrivate * priv = XDK_DISPLAY(object)->priv;
	
	switch(property_id) {
	case PROP_PEER:
		priv->peer = g_value_get_pointer(value);
		break;
	case PROP_DISPLAY_STRING:
		priv->display_string = g_value_get_string(value)
			? g_value_dup_string(value)
			: g_strdup(g_getenv("DISPLAY"));
		break;
	case PROP_EVENT_RETRIEVAL_DISABLED:
		priv->event_retrieval_disabled = g_value_get_boolean(value);
		break;
	}
}

gboolean xdk_display_open(XdkDisplay * self)
{
	g_return_val_if_fail(self, FALSE);
	
	XdkDisplayPrivate * priv = self->priv;
	
	if(! priv->peer) {
		priv->peer = XOpenDisplay(priv->display_string);
		priv->own_peer = TRUE;
	}
	if(! priv->peer) {
		goto end;
	}
	
	priv->n_screens = XScreenCount(priv->peer);
	
	int i;
	for(i = 0; i < priv->n_screens; i ++) {
		g_ptr_array_add(
			priv->screens,
			g_object_new(
				XDK_TYPE_SCREEN,
				"peer", XScreenOfDisplay(priv->peer, i),
				"display", self,
				NULL));
	}

	if(! priv->event_retrieval_disabled) {
		xdk_display_add_watch(self);
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

	goto end;
	
close_display:
	XCloseDisplay(priv->peer);
	priv->peer = NULL;
end:
	return NULL != priv->peer;
}

static void xdk_display_dispose(GObject * object)
{
	XdkDisplay * self = XDK_DISPLAY(object);
	XdkDisplayPrivate * priv = self->priv;
	
	xdk_display_remove_watch(self);
	
	g_hash_table_remove_all(priv->windows);
	
	g_ptr_array_remove_range(priv->screens, 0, priv->screens->len);
}

static void xdk_display_finalize(GObject * object)
{
	XdkDisplayPrivate * priv = XDK_DISPLAY(object)->priv;
	
	if(priv->display_string) {
		g_free(priv->display_string);
	}
	
	g_hash_table_unref(priv->windows);
	
	g_free(priv->screens);
	
	if(priv->peer && priv->own_peer) {
		XCloseDisplay(priv->peer);
	}
	
	G_OBJECT_CLASS(xdk_display_parent_class)->finalize(object);
}

XdkDisplay * xdk_display_get_default()
{
	if(G_UNLIKELY(! xdk_default_display)) {
		g_error("Call xdk_init() to initialize Xdk first");
	}
	
	return (XdkDisplay *) xdk_default_display;
}

Display * xdk_display_get_peer(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->peer;
}

const gchar * xdk_display_get_display_string(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->display_string;
}

gint xdk_display_get_n_screens(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->n_screens;
}

GList * xdk_display_list_screens(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	GPtrArray * screens = self->priv->screens;
	GList * new_screens = NULL;
	gint i;
	for(i = 0; i < screens->len; i ++) {
		new_screens = g_list_prepend(new_screens, g_ptr_array_index(screens, i));
	}
	
	return new_screens;
}

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self)
{
	return xdk_display_get_screen(self, DefaultScreen(self->priv->peer));
}

XdkScreen * xdk_display_get_screen(XdkDisplay * self, gint screen_number)
{
	g_return_val_if_fail(self, NULL);

	return g_ptr_array_index(self->priv->screens, screen_number);
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
	
	gchar * orig = XGetAtomName(self->priv->peer, atom);
	gchar * retval = g_strdup(orig);
	XFree(orig);
	
	return retval;
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
	XdkDisplay * self,
	XdkWindow * window)
{
	xdk_display_remove_window(self, window);
}

void xdk_display_add_window(XdkDisplay * self, XdkWindow * window)
{
	g_return_if_fail(self && window);
	Window xwin = xdk_window_get_peer(window);
	g_return_if_fail(None != xwin);
	
	g_signal_connect_data(
		window, "destroy",
		G_CALLBACK(xdk_display_window_destroyed), self,
		NULL,
		G_CONNECT_AFTER | G_CONNECT_SWAPPED);
	
	g_hash_table_insert(
		self->priv->windows,
		GUINT_TO_POINTER(xwin),
		g_object_ref(window));
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
	
	g_signal_handlers_disconnect_by_func(
		window,
		xdk_display_window_destroyed,
		self);
	g_hash_table_remove(self->priv->windows, GUINT_TO_POINTER(xwin));
}

int xdk_display_get_connection_number(XdkDisplay * self)
{
	g_return_val_if_fail(self, -1);
	
	return XConnectionNumber(self->priv->peer);
}

gboolean xdk_display_handle_event(XdkDisplay * self, XEvent * event)
{
	g_return_if_fail(self);

	XdkDisplayPrivate * priv = self->priv;
	GList * node = g_list_last(priv->event_filters);
	while(node) {
		XdkEventFilterNode * filter_node = XDK_EVENT_FILTER_NODE(node->data);
		if(filter_node->filter(self, event, filter_node->user_data)) {
			return TRUE;
		}
		node = g_list_previous(node);
	}
	
	XdkWindow * window = xdk_display_lookup_window(
		self,
		event->xany.window);
	if(! window) {
		return FALSE;
	}
	
	xdk_window_dispatch_event(window, event);
	
	return TRUE;
}

static void xdk_display_fetch_and_handle_event(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XEvent event;
	XNextEvent(self->priv->peer, & event);
	xdk_display_handle_event(self, & event);
}

static gboolean xdk_display_source_prepare(GSource * source, gint * timeout)
{
	* timeout = -1;
	
	return XPending(XDK_DISPLAY_SOURCE(source)->display->priv->peer);
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
	g_return_if_fail(self && self->priv->peer);
	
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
	priv->event_watch_id = 0;
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
		(GSourceFunc) xdk_display_fetch_and_handle_event, self,
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
		screens = g_list_append(screens, g_ptr_array_index(self->priv->screens, i));
	}
	
	return screens;
}

XdkScreen * xdk_display_lookup_screen(XdkDisplay * self, Screen * screen)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(screen, NULL);
	
	XdkDisplayPrivate * priv = self->priv;
	gint screen_number = XScreenNumberOfScreen(screen);
	
	if(screen_number <= 0 || screen_number >= priv->screens->len) {
		return NULL;
	}
	
	return g_ptr_array_index(priv->screens, screen_number);
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
	
	return FALSE;
}

static int xdk_display_default_error_handler(Display * display, XErrorEvent * error)
{
	g_return_val_if_fail(! xdk_last_error, 0);

	xdk_last_error = xdk_error_new(error);
	if(xdk_last_error) {
		g_warning("%s", xdk_last_error->message);
		g_error_free(xdk_last_error);
		xdk_last_error = NULL;
	}
	
	return 1;
}

static int xdk_display_on_error(Display * display, XErrorEvent * error)
{
	g_return_val_if_fail(! xdk_last_error, 0);
	
	xdk_last_error = xdk_error_new(error);
}

void xdk_trap_error()
{
	XSetErrorHandler(xdk_display_on_error);
}

gint xdk_untrap_error(GError ** error)
{
	XdkDisplay * display = xdk_display_get_default();
	XSync(xdk_display_get_peer(display), FALSE);
	
	if(xdk_dump_error) {
		XSetErrorHandler(xdk_display_default_error_handler);
	}
	
	gint error_code = XDK_ERROR_SUCCESS;
	if(xdk_last_error) {
		error_code = xdk_last_error->code;
		g_propagate_error(error, xdk_last_error);
		xdk_last_error = NULL;
	}
	
	return error_code;
}

Display * xdk_get_default_xdisplay()
{
	return xdk_display_get_default()->priv->peer;
}

int xdk_display_grab_server(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return XGrabServer(self->priv->peer);
}

int xdk_display_ungrab_server(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return XUngrabServer(self->priv->peer);
}

gboolean xdk_display_has_composite_extension(XdkDisplay * self)
{
	g_return_val_if_fail(self, FALSE);
	
	int event_base, error_base;
	return XCompositeQueryExtension(self->priv->peer, & event_base, & error_base);
}

gboolean xdk_display_has_damage_extension(XdkDisplay * self)
{
	g_return_val_if_fail(self, FALSE);
	
	int event_base, error_base;
	return XDamageQueryExtension(self->priv->peer, & event_base, & error_base);
}

Cursor xdk_display_create_font_cursor(XdkDisplay * self, guint shape)
{
	g_return_val_if_fail(self, None);
	
	return XCreateFontCursor(self->priv->peer, shape);
}

void xdk_display_free_cursor(XdkDisplay * self, Cursor cursor)
{
	g_return_if_fail(self);
	g_return_if_fail(None != cursor);
	
	XFreeCursor(self->priv->peer, cursor);
}
