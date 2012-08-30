#include "xdk-display.h"
#include "xdk-window.h"
#include "xdk-visual.h"
#include "xdk-gc.h"
#include "xdk-screen-private.h"

#define XDK_SCREEN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE(o, XDK_TYPE_SCREEN, XdkScreenPrivate))

struct _XdkScreenPrivate
{
	Screen * peer;
	
	XdkDisplay * display;
	
	XdkVisual * default_visual;
	
	XdkGc * default_gc;
	
	XdkWindow * root;
};

static void xdk_screen_dispose(GObject * object);

static void xdk_screen_finalize(GObject * object);

G_DEFINE_TYPE(XdkScreen, xdk_screen, G_TYPE_OBJECT);

static void xdk_screen_class_init(XdkScreenClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->dispose = xdk_screen_dispose;
	gobject_class->finalize = xdk_screen_finalize;
	
	g_type_class_add_private(clazz, sizeof(XdkScreenPrivate));
}

static void xdk_screen_init(XdkScreen * self)
{
	self->priv = XDK_SCREEN_GET_PRIVATE(self);
}

static void xdk_screen_dispose(GObject * object)
{
	XdkScreenPrivate * priv = XDK_SCREEN(object)->priv;
	
	if(priv->root) {
		g_object_unref(priv->root);
		priv->root = NULL;
	}
}

static void xdk_screen_finalize(GObject * object)
{
	G_OBJECT_CLASS(xdk_screen_parent_class)->finalize(object);
}

XdkScreen * xdk_screen_new()
{
	return g_object_new(XDK_TYPE_SCREEN, NULL);
}

void xdk_screen_set_display(XdkScreen * self, XdkDisplay * display)
{
	g_return_if_fail(self && display);
	
	self->priv->display = display;
}

void xdk_screen_set_peer(XdkScreen * self, Screen * peer)
{
	g_return_if_fail(self);
	g_return_if_fail(peer);
	
	self->priv->peer = peer;
}

Screen * xdk_screen_get_peer(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->peer;
}

gint xdk_screen_get_number(XdkScreen * self)
{
	g_return_val_if_fail(self, -1);
	g_return_val_if_fail(self->priv->peer, -1);
	
	return XScreenNumberOfScreen(self->priv->peer);
}

gint xdk_screen_get_width(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->priv->peer, 0);
	
	return XWidthOfScreen(self->priv->peer);
}

gint xdk_screen_get_height(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->priv->peer, 0);
	
	return HeightOfScreen(self->priv->peer);
}

gint xdk_screen_get_default_depth(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->priv->peer, 0);
	
	return XDefaultDepthOfScreen(self->priv->peer);
}

XdkGc * xdk_screen_get_default_gc(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->default_gc;
}

XdkVisual * xdk_screen_get_default_visual(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->default_visual;
}

XdkDisplay * xdk_screen_get_display(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->display;
}

glong xdk_screen_get_event_mask(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	
	return XEventMaskOfScreen(self->priv->peer);
}

XdkWindow * xdk_screen_get_root_window(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	XdkScreenPrivate * priv = self->priv;
	
	if(! priv->root) {
		Window xroot = XRootWindowOfScreen(priv->peer);
		priv->root = xdk_window_new();
		xdk_window_set_foreign_peer(priv->root, xroot);
		g_message("xdk_screen_get_root_window %p %x", priv->root, xroot);
	}
	
	return priv->root;
}
