#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-visual.h"

#define XDK_VISUAL_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE(o, XDK_TYPE_VISUAL, XdkVisualPrivate))

struct _XdkVisualPrivate
{
	Visual * peer;
	
	guint depth;
	
	gulong red_mask;
	
	gulong green_mask;
	
	gulong blue_mask;
	
	gint colormap_size;
	
	XdkScreen * screen;
	
	Colormap colormap;
};

enum
{
	PROP_INFO = 1,
	PROP_SCREEN
};

static void xdk_visual_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec);
	
static void xdk_visual_finalize(GObject * object);
	
G_DEFINE_TYPE(XdkVisual, xdk_visual, G_TYPE_OBJECT);

void xdk_visual_class_init(XdkVisualClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->set_property = xdk_visual_set_property;
	gobject_class->finalize = xdk_visual_finalize;

	g_object_class_install_property(
			gobject_class,
			PROP_INFO,
			g_param_spec_pointer(
				"info", "", "",
				G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
		
	g_object_class_install_property(
			gobject_class,
			PROP_SCREEN,
			g_param_spec_object(
				"screen", "", "",
				XDK_TYPE_SCREEN,
				G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
		
	g_type_class_add_private(clazz, sizeof(XdkVisualPrivate));
}

void xdk_visual_init(XdkVisual * self)
{
	self->priv = XDK_VISUAL_GET_PRIVATE(self);
	self->priv->colormap = None;
}

static void xdk_visual_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec)
{
	XdkVisual * self = XDK_VISUAL(object);
	XdkVisualPrivate * priv = self->priv;
	
	switch(property_id) {
	case PROP_INFO: {
		XVisualInfo * info = g_value_get_pointer(value);
		priv->peer = info->visual;
		priv->depth = info->depth;
		priv->red_mask = info->red_mask;
		priv->green_mask = info->green_mask;
		priv->blue_mask = info->blue_mask;
		priv->colormap_size = info->colormap_size;
		break;
	}
	case PROP_SCREEN:
		self->priv->screen = g_value_get_object(value);
		break;
	default:
		g_return_if_reached();
	}
}

XdkVisual * xdk_visual_new(XVisualInfo * info, XdkScreen * screen)
{
	g_return_val_if_fail(info, NULL);
	g_return_val_if_fail(screen, NULL);
	
	return g_object_new(
		XDK_TYPE_VISUAL,
		"info", info,
		"screen", screen,
		NULL);
}

static void xdk_visual_finalize(GObject * object)
{
	XdkVisualPrivate * priv = XDK_VISUAL(object)->priv;
	if(None != priv->colormap) {
		XFreeColormap(
			xdk_display_get_peer(xdk_screen_get_display(priv->screen)),
			priv->colormap);
	}
}

Visual * xdk_visual_get_peer(XdkVisual * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->peer;
}

VisualID xdk_visual_get_id(XdkVisual * self)
{
	g_return_val_if_fail(self, 0);
	
	return XVisualIDFromVisual(self->priv->peer);
}

gint xdk_visual_get_depth(XdkVisual * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->depth;
}

XdkVisualType xdk_visual_get_visual_type(XdkVisual * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->peer->class;
}

gulong xdk_visual_get_red_mask(XdkVisual * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->red_mask;
}

gulong xdk_visual_get_green_mask(XdkVisual * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->green_mask;
}

gulong xdk_visual_get_blue_mask(XdkVisual * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->blue_mask;
}

gint xdk_visual_get_colormap_size(XdkVisual * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->colormap_size;
}

Colormap xdk_visual_to_colormap(XdkVisual * self)
{
	g_return_val_if_fail(self, None);
	
	XdkVisualPrivate * priv = self->priv;
	if(priv->colormap) {
		goto end;
	}
	
	priv->colormap = XCreateColormap(
		xdk_display_get_peer(xdk_screen_get_display(priv->screen)),
		xdk_window_get_peer(xdk_screen_get_root_window(priv->screen)),
		priv->peer,
		AllocNone);
	XInstallColormap(
		xdk_display_get_peer(xdk_screen_get_display(priv->screen)),
		priv->colormap);
	
end:
	return priv->colormap;
}
