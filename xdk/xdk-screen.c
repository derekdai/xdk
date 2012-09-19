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
	
	XdkWindow * root;
	
	GHashTable * visuals;
	
	XdkVisual * default_visual;
	
	XdkVisual * argb_visual;
};

enum {
	PROP_PEER = 1,
	PROP_DISPLAY
};

static void xdk_screen_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec);

static void xdk_screen_constructed(GObject * object);

static void xdk_screen_dispose(GObject * object);

static void xdk_screen_finalize(GObject * object);

G_DEFINE_TYPE(XdkScreen, xdk_screen, G_TYPE_OBJECT);

static void xdk_screen_class_init(XdkScreenClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->dispose = xdk_screen_dispose;
	gobject_class->finalize = xdk_screen_finalize;
	gobject_class->set_property = xdk_screen_set_property;
	gobject_class->constructed = xdk_screen_constructed;
	
	g_object_class_install_property(
		gobject_class,
		PROP_PEER,
		g_param_spec_pointer(
			"peer", "", "",
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	
	g_object_class_install_property(
		gobject_class,
		PROP_DISPLAY,
		g_param_spec_object(
			"display", "", "",
			XDK_TYPE_DISPLAY,
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	
	g_type_class_add_private(clazz, sizeof(XdkScreenPrivate));
}

static void xdk_screen_init(XdkScreen * self)
{
	XdkScreenPrivate * priv = XDK_SCREEN_GET_PRIVATE(self);
	self->priv = priv;
	
	priv->visuals = g_hash_table_new_full(
		g_direct_hash, g_direct_equal,
		NULL, (GDestroyNotify) g_object_unref);
}

static void xdk_screen_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec)
{
	switch(property_id) {
	case PROP_PEER:
		XDK_SCREEN(object)->priv->peer = g_value_get_pointer(value);
		break;
	case PROP_DISPLAY:
		XDK_SCREEN(object)->priv->display = g_value_get_object(value);
		break;
	default:
		g_return_if_reached();
	}
}

static void xdk_screen_constructed(GObject * object)
{
	G_OBJECT_CLASS(xdk_screen_parent_class)->constructed(object);
	
	XdkScreen * self = XDK_SCREEN(object);
	XdkScreenPrivate * priv = self->priv;
	
	// enumerate all visuals
	int n_visual_infos;
	XVisualInfo * visual_infos = XGetVisualInfo(
		xdk_display_get_peer(priv->display),
		0, NULL,
		& n_visual_infos);
	if(! visual_infos) {
		g_warning("No visual information found for screen %d",
			xdk_screen_get_number(self));
		return;
	}
	
	Visual * default_visual = DefaultVisualOfScreen(priv->peer);
	
	int screen_number = xdk_screen_get_number(self);
	for(-- n_visual_infos; n_visual_infos >= 0; n_visual_infos --) {
		if(screen_number != visual_infos[n_visual_infos].screen) {
			continue;
		}
		
		XdkVisual * visual = xdk_visual_new(
			& visual_infos[n_visual_infos],
			self);
		g_hash_table_insert(
			priv->visuals,
			visual_infos[n_visual_infos].visual,
			visual);
			
		if(! priv->argb_visual &&
			TrueColor == visual_infos[n_visual_infos].class &&
			32 == visual_infos[n_visual_infos].depth &&
			0xff0000 == visual_infos[n_visual_infos].red_mask &&
				0x00ff00 == visual_infos[n_visual_infos].green_mask &&
				0x0000ff == visual_infos[n_visual_infos].blue_mask) {
			priv->argb_visual = visual;
		}
		
		if(default_visual == visual_infos[n_visual_infos].visual) {
			priv->default_visual = visual;
		}
	}
	
	XFree(visual_infos);
	
	// attach with root window
	priv->root = g_object_new(XDK_TYPE_WINDOW, "screen", object, NULL);
	xdk_window_set_foreign_peer(
		priv->root,
		RootWindowOfScreen(priv->peer));
}

static void xdk_screen_dispose(GObject * object)
{
	XdkScreenPrivate * priv = XDK_SCREEN(object)->priv;
	
	if(priv->root) {
		g_object_unref(priv->root);
		priv->root = NULL;
	}
	
	g_hash_table_remove_all(priv->visuals);
}

static void xdk_screen_finalize(GObject * object)
{
	XdkScreenPrivate * priv = XDK_SCREEN(object)->priv;
	
	g_hash_table_unref(priv->visuals);
	
	G_OBJECT_CLASS(xdk_screen_parent_class)->finalize(object);
}

void _xdk_screen_set_display(XdkScreen * self, XdkDisplay * display)
{
	g_return_if_fail(self && display);
	
	self->priv->display = display;
}

void _xdk_screen_set_peer(XdkScreen * self, Screen * peer)
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
	g_return_val_if_fail(self->priv->peer, 0);
	
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
	
	return xdk_visual_get_depth(self->priv->default_visual);
}

XdkDisplay * xdk_screen_get_display(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->display;
}

XdkWindow * xdk_screen_get_root_window(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	XdkScreenPrivate * priv = self->priv;
	
	if(! priv->root) {
		priv->root = xdk_window_new();
		xdk_window_set_foreign_peer(
			priv->root,
			XRootWindowOfScreen(priv->peer));
	}
	
	return priv->root;
}

gulong xdk_screen_get_white(XdkScreen * self)
{
	g_return_val_if_fail(self, ~ (gulong) 0);
	
	return WhitePixelOfScreen(self->priv->peer);
}

gulong xdk_screen_get_black(XdkScreen * self)
{
	g_return_val_if_fail(self, (gulong) 0);
	
	return BlackPixelOfScreen(self->priv->peer);
}

gboolean xdk_screen_is_use_backing_store(XdkScreen * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return NotUseful != DoesBackingStore(self->priv->peer);
}

gboolean xdk_screen_is_save_unders(XdkScreen * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return True == XDoesSaveUnders(self->priv->peer);
}

XdkVisual * xdk_screen_get_default_visual(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->default_visual;
}

XdkVisual * xdk_screen_get_rgba_visual(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->argb_visual;
}

XdkVisual * xdk_screen_lookup_visual(XdkScreen * self, Visual * visual)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(visual, NULL);
	
	return g_hash_table_lookup(self->priv->visuals, visual);
}

XdkVisual * xdk_screen_lookup_visual_by_id(XdkScreen * self, VisualID id)
{
	g_return_val_if_fail(self, NULL);
	
	Visual * visual = _XVIDtoVisual(self->priv->peer, id);
	if(! visual) {
		return NULL;
	}
	
	return xdk_screen_lookup_visual(self, visual);
}

GList * xdk_screen_list_visuals(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return g_hash_table_get_values(self->priv->visuals);
}
