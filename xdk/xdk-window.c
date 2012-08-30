#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-base-private.h"
#include "xdk-window-private.h"

gboolean xdk_window_init(gpointer base)
{
	g_return_val_if_fail(base, FALSE);
	
	XdkWindow * self = XDK_WINDOW(base);
	self->display = xdk_display_get_default();
	self->screen = xdk_display_get_default_screen(self->display);
	self->peer = None;
	self->background_color = 0xffffffff;
	self->width = 1;
	self->height = 1;
	
	return TRUE;
}

XdkWindow * xdk_window_new()
{
	return xdk_base_new(XDK_TYPE_WINDOW);
}

static void _xdk_window_set_peer(XdkWindow * self, Window peer, gboolean own_peer)
{
	g_return_if_fail(self);
	g_return_if_fail(! self->peer && None != peer);
	
	self->peer = peer;
	self->own_peer = own_peer;
	
	xdk_display_add_window(self->display, self);
}

void xdk_window_set_foreign_peer(XdkWindow * self, Window peer)
{
	_xdk_window_set_peer(self, peer, FALSE);
}

Window xdk_window_get_peer(XdkWindow * self)
{
	g_return_val_if_fail(self, None);
	
	return self->peer;
}

gboolean xdk_window_is_realized(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return None != self->peer;
}

/**
 * http://tronche.com/gui/x/xlib/window/XCreateWindow.html
 */
void xdk_window_realize(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(None != self->peer || self->destroyed) {
		return;
	}
	
	XdkWindow * parent = self->parent_window;
	if(! parent) {
		parent = xdk_get_default_root_window();
	}
	
	g_message("xdk_window_realize %p %x", parent, xdk_window_get_peer(parent));
	
	Window peer = XCreateSimpleWindow(
		xdk_display_get_peer(self->display),
		xdk_window_get_peer(parent),
		self->x, self->y,
		self->width, self->height,
		0, 0,
		self->background_color);
	
	_xdk_window_set_peer(self, peer, TRUE);
	
	xdk_display_flush(self->display);
}

void xdk_window_unrealize(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(None == self->peer || ! self->own_peer) {
		return;
	}
	
	xdk_display_remove_window(self->display, self);
	
	xdk_window_unmap(self);
	
	XDestroyWindow(xdk_display_get_peer(self->display), self->peer);
	self->peer = None;
}

void xdk_window_get_position(XdkWindow * self, int * x, int * y)
{
	g_return_if_fail(self);
	
	if(x) {
		* x = self->x;
	}
	
	if(y) {
		* y = self->y;
	}
}

void xdk_window_set_position(XdkWindow * self, int x, int y)
{
	g_return_if_fail(self);
	
	self->x = x;
	self->y = y;
}

void xdk_window_get_size(XdkWindow * self, int * width, int * height)
{
	g_return_if_fail(self);

	if(width) {
		* width = self->width;
	}
	
	if(height) {
		* height = self->height;
	}
}

void xdk_window_set_size(XdkWindow * self, int width, int height)
{
	g_return_if_fail(self);
	
	self->width = width;
	self->height = height;
}

void xdk_window_map(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(self->mapped || self->destroyed) {
		return;
	}
	
	xdk_window_realize(self);

	XMapWindow(xdk_display_get_peer(self->display), self->peer);
	self->mapped = TRUE;
}

gboolean xdk_window_is_mapped(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return self->mapped;
}

void xdk_window_unmap(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! xdk_window_is_realized(self) || ! self->mapped) {
		return;
	}
	
	XUnmapWindow(xdk_display_get_peer(self->display), self->peer);
	self->mapped = FALSE;
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
	
	return self->destroyed;
}

void xdk_window_destroy(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! self->own_peer) {
		return;
	}
	
	xdk_window_unmap(self);
	xdk_window_unrealize(self);
	
	self->destroyed = TRUE;
}

Atom * xdk_window_list_properties(XdkWindow * self, int * n_props)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(n_props, NULL);
	
	xdk_window_realize(self);
	
	return XListProperties(
		xdk_display_get_peer(self->display),
		self->peer,
		n_props);
}

void xdk_window_handle_event(XdkWindow * self, XEvent * event)
{
}

static void xdk_window_finalize(XdkWindow * self)
{
	xdk_window_destroy(self);
}

const XdkTypeInfo xdk_type_window = {
	XDK_TYPE_BASE,
	"XdkWindow",
	sizeof(XdkWindow),
	xdk_window_init,
	(XdkFinalizeFunc) xdk_window_finalize,
};
