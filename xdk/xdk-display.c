#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-screen-private.h"

#define XDK_DISPLAY_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE(o, XDK_TYPE_DISPLAY, XdkDisplayPrivate))

static XdkDisplay * default_display = NULL;

struct _XdkDisplayPrivate
{
	Display * peer;
	
	gchar * name;
	
	gint n_screens;
	
	gint default_screen;
	
	XdkScreen ** screens;
	
	GHashTable * windows;
	
	XEvent xevent;
};

G_DEFINE_TYPE(XdkDisplay, xdk_display, G_TYPE_OBJECT);

static void xdk_display_dispose(GObject * object);

static void xdk_display_finalize(GObject * object);

void xdk_display_class_init(XdkDisplayClass * clazz)
{
	GObjectClass * gobject_clazz = G_OBJECT_CLASS(clazz);
	gobject_clazz->dispose = xdk_display_dispose;
	gobject_clazz->finalize = xdk_display_finalize;
	
	g_type_class_add_private(clazz, sizeof(XdkDisplayPrivate));
}

gboolean xdk_display_init_once()
{
	if(! default_display) {
		g_object_new(XDK_TYPE_DISPLAY, NULL);
	}
	
	return NULL != default_display;
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
	g_message("self->default_screen=%d", priv->default_screen);
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

XdkDisplay * xdk_display_new()
{
	return g_object_new(XDK_TYPE_DISPLAY, NULL);
}

XdkDisplay * xdk_display_get_default()
{
	if(! default_display) {
		g_error("Call xdk_init() to initialize first");
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

gboolean xdk_display_next_event(XdkDisplay * self)
{
	g_return_val_if_fail(self, FALSE);
	
	XNextEvent(self->priv->peer, & self->priv->xevent);
	
	return TRUE;
}

void xdk_display_dispatch_event(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XdkWindow * window = xdk_display_lookup_window(self, self->priv->xevent.xany.window);
	if(! window) {
		return;
	}
	
	xdk_window_handle_event(window, & self->priv->xevent);
}
