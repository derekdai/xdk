#ifndef __XDK_VISUAL_H_
#define __XDK_VISUAL_H_

#include <glib-object.h>
#include <X11/Xutil.h>

G_BEGIN_DECLS

#define XDK_VISUAL(o)			(XDK_CAST(o, XdkVisual, XDK_TYPE_VISUAL))

typedef struct _XdkVisual XdkVisual;

typedef enum _XdkVisualTypes XdkVisualTypes;

typedef enum _XdkVisualInfoMask XdkVisualInfoMask;

/**
 * http://tronche.com/gui/x/xlib/window/visual-types.html
 * http://tronche.com/gui/x/xlib/utilities/visual.html
 */
enum _XdkVisualTypes
{
	XDK_VISUAL_TYPES_PSEUDO_COLOR	= PseudoColor,
	XDK_VISUAL_TYPES_GRAY_SCALE		= GrayScale,
	XDK_VISUAL_TYPES_DIRECT_COLOR	= DirectColor,
	XDK_VISUAL_TYPES_TRUE_COLOR		= TrueColor,
	XDK_VISUAL_TYPES_STATIC_COLOR	= StaticColor,
	XDK_VISUAL_TYPES_STATIC_GRAY	= StaticGray,
};

enum _XdkVisualInfoMask
{
	XDK_VISUAL_INFO_MASK_NO				= VisualNoMask,
	XDK_VISUAL_INFO_MASK_ID				= VisualIDMask,
	XDK_VISUAL_INFO_MASK_SCREEN			= VisualScreenMask,
	XDK_VISUAL_INFO_MASK_DEPTH			= VisualDepthMask,
	XDK_VISUAL_INFO_MASK_CLASS			= VisualClassMask,
	XDK_VISUAL_INFO_MASK_RED_MASK 		= VisualRedMaskMask,
	XDK_VISUAL_INFO_MASK_GREEN_MASK 	= VisualGreenMaskMask,
	XDK_VISUAL_INFO_MASK_BLUE_MASK		= VisualBlueMaskMask,
	XDK_VISUAL_INFO_MASK_COLORMAP_SIZE	= VisualColormapSizeMask,
	XDK_VISUAL_INFO_MASK_BITS_PER_RGB	= VisualBitsPerRGBMask,
	XDK_VISUAL_INFO_MASK_ALL 			= VisualAllMask,
};

struct _XdkVisual
{
	
};

G_END_DECLS

#endif /* __XDK_VISUAL_H_ */
