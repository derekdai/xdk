#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-screen-private.h"

#define XDK_DISPLAY_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE(o, XDK_TYPE_DISPLAY, XdkDisplayPrivate))

#define XDK_DISPLAY_SOURCE(o) ((XdkDisplaySource *) (o))

typedef struct _XdkDisplaySource XdkDisplaySource;

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
};

struct _XdkDisplaySource
{
	GSource base;
	
	XdkDisplay * display;
};

enum {
	SIGNAL_ERROR = 0,
	SIGNAL_MAX,
};

static void xdk_display_dispose(GObject * object);

static void xdk_display_finalize(GObject * object);

static void xdk_display_error(
	XdkDisplay * self,
	XErrorEvent * error);

G_DEFINE_TYPE(XdkDisplay, xdk_display, G_TYPE_OBJECT);

static guint signals[SIGNAL_MAX] = { 0, };

static XdkDisplay * default_display = NULL;

void xdk_display_class_init(XdkDisplayClass * clazz)
{
	GObjectClass * gobject_clazz = G_OBJECT_CLASS(clazz);
	gobject_clazz->dispose = xdk_display_dispose;
	gobject_clazz->finalize = xdk_display_finalize;
	
	signals[SIGNAL_ERROR] = g_signal_new(
		"error",
		G_TYPE_FROM_CLASS(clazz),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkDisplayClass, error),
		NULL, NULL,
		g_cclosure_marshal_VOID__POINTER,
		G_TYPE_NONE,
		1, G_TYPE_POINTER);
		
	clazz->error = xdk_display_error;
	
	g_type_class_add_private(clazz, sizeof(XdkDisplayPrivate));
}

static int on_x_error(Display * display, XErrorEvent * error)
{
	g_error("Error");
}

static void xdk_display_init(XdkDisplay * self)
{
	XdkDisplayPrivate * priv = XDK_DISPLAY_GET_PRIVATE(self);
	self->priv = priv;

	gboolean result = FALSE;
	priv->peer = XOpenDisplay(NULL);
	if(! priv->peer) {
		g_error("Failed to initialize display");
	}
	
	priv->name = XDisplayString(priv->peer);
	priv->default_screen = XDefaultScreen(priv->peer);
	priv->n_screens = XScreenCount(priv->peer);
	priv->screens = g_malloc0(sizeof(XdkScreen *) * priv->n_screens);
	priv->windows = g_hash_table_new_full(
		g_direct_hash,
		g_direct_equal,
		NULL,
		g_object_unref
	);
	
	// display ready to expose to outside world
	default_display = self;

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
	
	XSetErrorHandler(on_x_error);
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

static void xdk_display_error(XdkDisplay * self, XErrorEvent * error)
{
}

XdkDisplay * xdk_display_get_default()
{
	if(! default_display) {
		g_object_new(XDK_TYPE_DISPLAY, NULL);
	}
	
	return default_display;
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

Atom xdk_atom_from_name(
	const char * atom_name,
	gboolean only_if_exists)
{
	g_return_val_if_fail(atom_name, None);
	XdkDisplay * display = xdk_display_get_default();
	g_return_val_if_fail(display, None);
	
	return XInternAtom(display->priv->peer, atom_name, only_if_exists);
}

gchar * xdk_atom_to_name(Atom atom)
{
	g_return_val_if_fail(None != atom, NULL);
	XdkDisplay * display = xdk_display_get_default();
	g_return_val_if_fail(display, None);
	
	return XGetAtomName(display->priv->peer, atom);
}

void xdk_display_add_window(XdkDisplay * self, XdkWindow * window)
{
	g_return_if_fail(self && window);
	Window xwin = xdk_window_get_peer(window);
	g_return_if_fail(None != xwin);
	
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

static void xdk_display_dispatch_event(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XEvent event;
	XNextEvent(self->priv->peer, & event);
	
	g_debug("xdk_display_dispatch_event: %d", event.type);
	
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
	
	return FALSE;
}

static gboolean xdk_display_source_check(GSource * source)
{
	int num_events = XPending(XDK_DISPLAY_SOURCE(source)->display->priv->peer);
	
	g_debug("xdk_display_source_check: %d", num_events);
	
	return num_events > 0;
}

static gboolean xdk_display_source_dispatch(
	GSource * source,
	GSourceFunc callback,
	gpointer user_data)
{
	callback(user_data);
}

int xdk_display_get_connection_number(XdkDisplay * self)
{
	g_return_val_if_fail(self, -1);
	
	return XConnectionNumber(self->priv->peer);
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
	g_source_attach(source, NULL);
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
	priv->poll_fd.events = G_IO_IN;
	
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
