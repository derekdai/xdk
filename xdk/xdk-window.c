#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-window-private.h"
#include "xdk-gc.h"
#include "xdk-visual.h"

#define XDK_WINDOW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE(o, XDK_TYPE_WINDOW, XdkWindowPrivate))

struct _XdkWindowPrivate
{
	Window peer;
	
	XdkDisplay * display;
	
	XdkScreen * screen;
	
	XdkWindow * parent_window;
	
	gint x;
	
	gint y;
	
	guint width;
	
	guint height;
	
	XdkVisual * visual;
	
	gulong background_color;
	
	gboolean mapped : 1;
	
	gboolean visible : 1;
	
	gboolean own_peer : 1;
	
	gboolean destroyed : 1;
};

static void xdk_window_dispose(GObject * object);

static void xdk_window_finalize(GObject * object);

G_DEFINE_TYPE(XdkWindow, xdk_window, G_TYPE_OBJECT);

static void xdk_window_class_init(XdkWindowClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->dispose = xdk_window_dispose;
	gobject_class->finalize = xdk_window_finalize;
	
	g_type_class_add_private(clazz, sizeof(XdkWindowPrivate));
}

static void xdk_window_init(XdkWindow * self)
{
	XdkWindowPrivate * priv = XDK_WINDOW_GET_PRIVATE(self);
	self->priv = priv;
	
	priv->display = xdk_display_get_default();
	priv->screen = xdk_display_get_default_screen(priv->display);
	priv->peer = None;
	priv->background_color = 0xffffffff;
	priv->width = 1;
	priv->height = 1;
}

static void xdk_window_dispose(GObject * object)
{
	XdkWindowPrivate * priv = XDK_WINDOW(object)->priv;
}

static void xdk_window_finalize(GObject * object)
{
	XdkWindow * self = XDK_WINDOW(object);
	
	xdk_window_destroy(self);
	
	G_OBJECT_CLASS(xdk_window_parent_class)->finalize(object);
}

XdkWindow * xdk_window_new()
{
	return g_object_new(XDK_TYPE_WINDOW, NULL);
}

static void _xdk_window_set_peer(XdkWindow * self, Window peer, gboolean own_peer)
{
	g_return_if_fail(self);
	XdkWindowPrivate * priv = self->priv;
	g_return_if_fail(! priv->peer && None != peer);
	
	priv->peer = peer;
	priv->own_peer = own_peer;
	
	xdk_display_add_window(priv->display, self);
}

void xdk_window_set_foreign_peer(XdkWindow * self, Window peer)
{
	_xdk_window_set_peer(self, peer, FALSE);
}

Window xdk_window_get_peer(XdkWindow * self)
{
	g_return_val_if_fail(self, None);
	
	return self->priv->peer;
}

gboolean xdk_window_is_realized(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return None != self->priv->peer;
}

/**
 * http://tronche.com/gui/x/xlib/window/XCreateWindow.html
 */
void xdk_window_realize(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	
	if(None != priv->peer || priv->destroyed) {
		return;
	}
	
	XdkWindow * parent = priv->parent_window;
	if(! parent) {
		parent = xdk_get_default_root_window();
	}
	
	g_message("xdk_window_realize %p %x", parent, xdk_window_get_peer(parent));
	
	Window peer = XCreateSimpleWindow(
		xdk_display_get_peer(priv->display),
		xdk_window_get_peer(parent),
		priv->x, priv->y,
		priv->width, priv->height,
		0, 0,
		priv->background_color);
	
	_xdk_window_set_peer(self, peer, TRUE);
	
	xdk_display_flush(priv->display);
}

void xdk_window_unrealize(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(None == priv->peer || ! priv->own_peer) {
		return;
	}
	
	xdk_display_remove_window(priv->display, self);
	
	xdk_window_unmap(self);
	
	XDestroyWindow(xdk_display_get_peer(priv->display), priv->peer);
	priv->peer = None;
}

void xdk_window_get_position(XdkWindow * self, int * x, int * y)
{
	g_return_if_fail(self);
	
	if(x) {
		* x = self->priv->x;
	}
	
	if(y) {
		* y = self->priv->y;
	}
}

void xdk_window_set_position(XdkWindow * self, int x, int y)
{
	g_return_if_fail(self);
	
	self->priv->x = x;
	self->priv->y = y;
}

void xdk_window_get_size(XdkWindow * self, int * width, int * height)
{
	g_return_if_fail(self);

	if(width) {
		* width = self->priv->width;
	}
	
	if(height) {
		* height = self->priv->height;
	}
}

void xdk_window_set_size(XdkWindow * self, int width, int height)
{
	g_return_if_fail(self);
	
	self->priv->width = width;
	self->priv->height = height;
}

void xdk_window_map(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(priv->mapped || priv->destroyed) {
		return;
	}
	
	xdk_window_realize(self);

	XMapWindow(xdk_display_get_peer(priv->display), priv->peer);
	priv->mapped = TRUE;
}

gboolean xdk_window_is_mapped(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return self->priv->mapped;
}

void xdk_window_unmap(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(! xdk_window_is_realized(self) || ! priv->mapped) {
		return;
	}
	
	XUnmapWindow(xdk_display_get_peer(priv->display), priv->peer);
	priv->mapped = FALSE;
}

void xdk_window_show(XdkWindow * self)
{
	g_return_if_fail(self);
	
	xdk_window_realize(self);
	xdk_window_map(self);
}

void xdk_window_hide(XdkWindow * self)
{
}

gboolean xdk_window_is_destroyed(XdkWindow * self)
{
	g_return_val_if_fail(self, TRUE);
	
	return self->priv->destroyed;
}

void xdk_window_destroy(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! self->priv->own_peer) {
		return;
	}
	
	xdk_window_unmap(self);
	xdk_window_unrealize(self);
	
	self->priv->destroyed = TRUE;
}

Atom * xdk_window_list_properties(XdkWindow * self, int * n_props)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(n_props, NULL);
	
	xdk_window_realize(self);
	
	return XListProperties(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer,
		n_props);
}

void xdk_window_handle_event(XdkWindow * self, XEvent * event)
{
}
