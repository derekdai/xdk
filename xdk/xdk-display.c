#include "xdk-display.h"
#include "xdk-base-private.h"
#include "xdk-screen.h"
#include "xdk-screen-private.h"

static XdkDisplay * default_display = NULL;

struct _XdkDisplay
{
	XdkBase parent;
	
	Display * peer;
	
	gchar * name;
	
	gint n_screens;
	
	gint default_screen;
	
	XdkScreen ** screens;
	
	GHashTable * windows;
	
	XEvent xevent;
};

gboolean xdk_display_init_once()
{
	if(! default_display) {
		xdk_base_new(XDK_TYPE_DISPLAY);
	}
	
	return NULL != default_display;
}

gboolean xdk_display_init(gpointer base)
{
	g_return_val_if_fail(base, FALSE);

	gboolean result = FALSE;
	XdkDisplay * self = XDK_DISPLAY(base);
	self->peer = XOpenDisplay(NULL);
	if(! self->peer) {
		goto end;
	}
	
	self->name = XDisplayString(self->peer);
	self->default_screen = XDefaultScreen(self->peer);
	g_message("self->default_screen=%d", self->default_screen);
	self->n_screens = XScreenCount(self->peer);
	self->screens = g_malloc0(sizeof(XdkScreen *) * self->n_screens);
	self->windows = g_hash_table_new_full(
		g_direct_hash,
		g_direct_equal,
		NULL,
		xdk_base_unref
	);
	
	// display ready to expose to outside world
	default_display = self;

	int i;
	for(i = 0; i < self->n_screens; i ++) {
		XdkScreen * screen = xdk_base_new(XDK_TYPE_SCREEN);
		if(! screen) {
			goto end;
		}
		xdk_screen_set_display(screen, self);
		xdk_screen_set_peer(screen, XScreenOfDisplay(self->peer, i));
		self->screens[i] = screen;
	}
	
	for(i = 0; i < self->n_screens; i ++) {
		xdk_display_add_window(
			self,
			xdk_screen_get_root_window(self->screens[i]));
	}
	
	result = TRUE;
	
end:
	return result;
}

void xdk_display_destroy(gpointer base)
{
	g_return_if_fail(base);
	
	XdkDisplay * self = XDK_DISPLAY(base);
	
	int i;
	for(i = 0; i < self->n_screens; i ++) {
		if(self->screens[i]) {
			xdk_base_unref(self->screens[i]);
		}
	}
	g_free(self->screens);
	
	if(self->peer) {
		XCloseDisplay(self->peer);
	}
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
	
	return self->peer;
}

const gchar * xdk_display_get_name(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->name;
}

gint xdk_display_get_n_screens(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->n_screens;
}

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->screens[self->default_screen];
}

void xdk_display_flush(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XFlush(self->peer);
}

const char * xdk_display_get_vendor(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return XServerVendor(self->peer);
}

gint xdk_display_get_release(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return XVendorRelease(self->peer);
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
	
	return XInternAtom(display->peer, atom_name, only_if_exists);
}

gchar * xdk_atom_to_name(Atom atom)
{
	g_return_val_if_fail(None != atom, NULL);
	XdkDisplay * display = xdk_display_get_default();
	g_return_val_if_fail(display, None);
	
	return XGetAtomName(display->peer, atom);
}

void xdk_display_add_window(XdkDisplay * self, XdkWindow * window)
{
	g_return_if_fail(self && window);
	Window xwin = xdk_window_get_peer(window);
	g_return_if_fail(None != xwin);
	
	g_hash_table_insert(
		self->windows,
		GUINT_TO_POINTER(xwin),
		window);
}

XdkWindow * xdk_display_lookup_window(XdkDisplay * self, Window xwindow)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(None != xwindow, NULL);
	
	return g_hash_table_lookup(self->windows, GUINT_TO_POINTER(xwindow));
}

gboolean xdk_display_has_window(XdkDisplay * self, Window xwindow)
{
	g_return_val_if_fail(self, FALSE);
	
	return g_hash_table_contains(self->windows, GUINT_TO_POINTER(xwindow));
}

void xdk_display_remove_window(XdkDisplay * self, XdkWindow * window)
{
	g_return_if_fail(self && window);
	Window xwin = xdk_window_get_peer(window);
	g_return_if_fail(None != xwin);
	
	g_hash_table_remove(self->windows, GUINT_TO_POINTER(xwin));
}

gboolean xdk_display_next_event(XdkDisplay * self)
{
	g_return_val_if_fail(self, FALSE);
	
	XNextEvent(self->peer, & self->xevent);
	
	return TRUE;
}

void xdk_display_dispatch_event(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XdkWindow * window = xdk_display_lookup_window(self, self->xevent.xany.window);
	if(! window) {
		return;
	}
	
	xdk_window_handle_event(window, & self->xevent);
}

const XdkTypeInfo xdk_type_display = {
	XDK_TYPE_BASE,
	"XdkDisplay",
	sizeof(XdkDisplay),
	xdk_display_init,
	xdk_display_destroy
};
