#ifndef __XDK_H_
#define __XDK_H_

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

G_BEGIN_DECLS

#define XDK_CASE(o, c, x)		((c *) o)
#define XDK_BASE(o)				(XDK_CASE(o, XdkBase, XDK_TYPE_BASE))
#define XDK_DISPLAY(o)			(XDK_CASE(o, XdkDisplay, XDK_TYPE_DISPLAY))

typedef gboolean (* XdkInitFunc)(gpointer base);

typedef void (* XdkDestroyFunc)(gpointer base);

typedef enum _XdkType XdkType;

typedef struct _XdkBase XdkBase;

typedef struct _XdkDisplay XdkDisplay;

typedef struct _XdkScreen XdkScreen;

enum _XdkType
{
	XDK_TYPE_INVALID,
	XDK_TYPE_BASE,
	XDK_TYPE_DISPLAY,
	XDK_TYPE_SCREEN,
	XDK_TYPE_MAX
};

gboolean xdk_init(int * argc, char ** args[]);

gpointer xdk_base_new(XdkType type);

gpointer xdk_base_ref(gpointer base);

void xdk_base_unref(gpointer base);

XdkDisplay * xdk_display_get_default();

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self);

G_END_DECLS

#endif /* __XDK_H_ */
