#include "xdk.h"
#include "xdk-base-private.h"
#include "xdk-screen-private.h"

void xdk_screen_set_peer(XdkScreen * self, Screen * peer)
{
	g_return_if_fail(self);
	g_return_if_fail(peer);
	
	self->peer = peer;
}

Screen * xdk_screen_get_peer(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->peer;
}

gint xdk_screen_get_number(XdkScreen * self)
{
	g_return_val_if_fail(self, -1);
	g_return_val_if_fail(self->peer, -1);
	
	return XScreenNumberOfScreen(self->peer);
}

gint xdk_screen_get_width(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->peer, 0);
	
	return XWidthOfScreen(self->peer);
}

gint xdk_screen_get_height(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->peer, 0);
	
	return HeightOfScreen(self->peer);
}

gint xdk_screen_get_default_depth(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->peer, 0);
	
	return XDefaultDepthOfScreen(self->peer);
}

XdkGc * xdk_screen_get_default_gc(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->default_gc;
}

XdkVisual * xdk_screen_get_default_visual(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->default_visual;
}

XdkDisplay * xdk_screen_get_display(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->display;
}

glong xdk_screen_get_event_mask(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	
	return XEventMaskOfScreen(self->peer);
}

XdkWindow * xdk_screen_get_root_window(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	if(! self->root) {
		self->root = xdk_base_new(XDK_TYPE_WINDOW);
		xdk_window_set_peer(self->root, XRootWindowOfScreen(self->peer));
	}
	
	return self->root;
}

