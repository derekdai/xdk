/**
 * The best way to get yourself understand Xlib, see
 *   http://tronche.com/gui/x/xlib
 */

#ifndef __XDK_H_
#define __XDK_H_

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

G_BEGIN_DECLS

#define XDK_CAST(o, c, x)		((c *) o)
#define XDK_BASE(o)				(XDK_CAST(o, XdkBase, XDK_TYPE_BASE))
#define XDK_DISPLAY(o)			(XDK_CAST(o, XdkDisplay, XDK_TYPE_DISPLAY))
#define XDK_SCREEN(o)			(XDK_CAST(o, XdkScreen, XDK_TYPE_SCREEN))
#define XDK_WINDOW(o)			(XDK_CAST(o, XdkWindow, XDK_TYPE_WINDOW))
#define XDK_GC(o)				(XDK_CAST(o, XdkGc, XDK_TYPE_GC))
#define XDK_VISUAL(o)			(XDK_CAST(o, XdkVisual, XDK_TYPE_VISUAL))

typedef gboolean (* XdkInitFunc)(gpointer base);

typedef void (* XdkDestroyFunc)(gpointer base);

typedef enum _XdkType XdkType;

typedef struct _XdkBase XdkBase;

typedef struct _XdkDisplay XdkDisplay;

typedef struct _XdkScreen XdkScreen;

typedef struct _XdkGc XdkGc;

typedef struct _XdkVisual XdkVisual;

typedef struct _XdkWindow XdkWindow;

enum _XdkType
{
	XDK_TYPE_INVALID,
	XDK_TYPE_BASE,
	XDK_TYPE_DISPLAY,
	XDK_TYPE_SCREEN,
	XDK_TYPE_WINDOW,
	XDK_TYPE_GC,
	XDK_TYPE_VSIUAL,
	XDK_TYPE_MAX
};

gboolean xdk_init(int * argc, char ** args[]);

gpointer xdk_base_new(XdkType type);

gpointer xdk_base_ref(gpointer base);

void xdk_base_unref(gpointer base);

XdkDisplay * xdk_display_get_default();

Display * xdk_display_get_peer(XdkDisplay * self);

const char * xdk_display_get_vendor(XdkDisplay * self);

gint xdk_display_get_release(XdkDisplay * self);

const gchar * xdk_display_get_name(XdkDisplay * self);

gint xdk_display_get_n_screens(XdkDisplay * self);

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self);

void xdk_display_flush(XdkDisplay * self);

XdkWindow * xdk_get_default_root_window();

Screen * xdk_screen_get_peer(XdkScreen * self);

gint xdk_screen_get_number(XdkScreen * self);

gint xdk_screen_get_width(XdkScreen * self);

gint xdk_screen_get_height(XdkScreen * self);

gint xdk_screen_get_default_depth(XdkScreen * self);

XdkGc * xdk_screen_get_default_gc(XdkScreen * self);

XdkVisual * xdk_screen_get_default_visual(XdkScreen * self);

XdkDisplay * xdk_screen_get_display(XdkScreen * self);

glong xdk_screen_get_event_mask(XdkScreen * self);

XdkWindow * xdk_screen_get_root_window(XdkScreen * self);

XdkWindow * xdk_window_new();

void xdk_window_set_peer(XdkWindow * self, Window peer);

Window xdk_window_get_peer(XdkWindow * self);

gboolean xdk_window_is_realized(XdkWindow * self);

void xdk_window_get_position(XdkWindow * self, int * x, int * y);

void xdk_window_set_position(XdkWindow * self, int x, int y);

void xdk_window_get_size(XdkWindow * self, int * width, int * height);

void xdk_window_set_size(XdkWindow * self, int width, int height);

G_END_DECLS

#endif /* __XDK_H_ */
