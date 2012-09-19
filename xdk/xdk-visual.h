#ifndef __XDK_VISUAL_H_
#define __XDK_VISUAL_H_

#include <glib-object.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

G_BEGIN_DECLS

#define XDK_TYPE_VISUAL (xdk_visual_get_type())
#define XDK_VISUAL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), XDK_TYPE_VISUAL, XdkVisual))
#define XDK_VISUAL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XDK_TYPE_VISUAL, XdkVisualClass))
#define IS_XDK_VISUAL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XDK_TYPE_VISUAL))
#define IS_XDK_VISUAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XDK_TYPE_VISUAL))
#define XDK_VISUAL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), XDK_TYPE_VISUAL, XdkVisualClass))

typedef struct _XdkVisualClass XdkVisualClass;

typedef struct _XdkVisual XdkVisual;

typedef struct _XdkVisualPrivate XdkVisualPrivate;

typedef enum _XdkVisualType XdkVisualType;

typedef enum _XdkVisualInfoMask XdkVisualInfoMask;

struct _XdkScreen;

/**
 * http://tronche.com/gui/x/xlib/window/visual-types.html
 * http://tronche.com/gui/x/xlib/utilities/visual.html
 */
enum _XdkVisualType
{
	XDK_VISUAL_TYPE_PSEUDO_COLOR	= PseudoColor,
	XDK_VISUAL_TYPE_GRAY_SCALE		= GrayScale,
	XDK_VISUAL_TYPE_DIRECT_COLOR	= DirectColor,
	XDK_VISUAL_TYPE_TRUE_COLOR		= TrueColor,
	XDK_VISUAL_TYPE_STATIC_COLOR	= StaticColor,
	XDK_VISUAL_TYPE_STATIC_GRAY		= StaticGray,
};

struct _XdkVisualClass
{
	GObjectClass base;
};

struct _XdkVisual
{
	GObject base;
	
	XdkVisualPrivate * priv;
};

GType xdk_visual_get_type();

XdkVisual * xdk_visual_new(XVisualInfo * visual_info, struct _XdkScreen * screen);

Visual * xdk_visual_get_peer(XdkVisual * self);

VisualID xdk_visual_get_id(XdkVisual * self);

gint xdk_visual_get_depth(XdkVisual * self);

XdkVisualType xdk_visual_get_visual_type(XdkVisual * self);

gulong xdk_visual_get_red_mask(XdkVisual * self);

gulong xdk_visual_get_green_mask(XdkVisual * self);

gulong xdk_visual_get_blue_mask(XdkVisual * self);

gint xdk_visual_get_colormap_size(XdkVisual * self);

Colormap xdk_visual_to_colormap(XdkVisual * self);

G_END_DECLS

#endif /* __XDK_VISUAL_H_ */
