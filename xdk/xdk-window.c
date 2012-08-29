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

void xdk_window_set_peer(XdkWindow * self, Window peer)
{
	g_return_if_fail(self);
	g_return_if_fail(! self->peer);
	
	self->peer = peer;
	self->own_peer = FALSE;
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
void xdk_window_realize_simple(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindow * parent = self->parent_window;
	if(! parent) {
		// 
		parent = xdk_get_default_root_window();
	}
	
	self->peer = XCreateSimpleWindow(
		xdk_display_get_peer(self->display),
		xdk_window_get_peer(parent),
		self->x, self->y,
		self->width, self->height,
		0, 0,
		self->background_color);
	self->own_peer = TRUE;
	
	xdk_display_flush();
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
	
	if(! self->mapped) {
		XMapWindow(xdk_display_get_peer(self->display), self->peer);
		self->mapped = TRUE;
	}
}

void xdk_window_show(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(None == self->peer) {
		xdk_window_realize_simple(self);
	}
	
	if(! self->mapped) {
		xdk_window_map(self);
	}
}

void xdk_window_hide(XdkWindow * self)
{
}

const XdkTypeInfo xdk_type_window = {
	XDK_TYPE_BASE,
	"XdkWindow",
	sizeof(XdkWindow),
	xdk_window_init,
	NULL
};
