#ifndef __XDK_SCREEN_H_
#define __XDK_SCREEN_H_

#include <glib.h>
#include <X11/Xlib.h>

G_BEGIN_DECLS

#define XDK_TYPE_SCREEN (xdk_screen_get_type())
#define XDK_SCREEN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), XDK_TYPE_SCREEN, XdkScreen))
#define XDK_SCREEN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XDK_TYPE_SCREEN, XdkScreenClass))
#define IS_XDK_SCREEN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XDK_TYPE_SCREEN))
#define IS_XDK_SCREEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XDK_TYPE_SCREEN))
#define XDK_SCREEN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), XDK_TYPE_SCREEN, XdkScreenClass))

typedef struct _XdkScreenClass XdkScreenClass;

typedef struct _XdkScreen XdkScreen;

typedef struct _XdkScreenPrivate XdkScreenPrivate;

struct _XdkScreenClass
{
	GObjectClass base;
};

struct _XdkScreen
{
	GObject base;
	
	XdkScreenPrivate * priv;
};

GType xdk_screen_get_type();

XdkScreen * xdk_screen_new();

Screen * xdk_screen_get_peer(XdkScreen * self);

gint xdk_screen_get_number(XdkScreen * self);

gint xdk_screen_get_width(XdkScreen * self);

gint xdk_screen_get_height(XdkScreen * self);

gint xdk_screen_get_default_depth(XdkScreen * self);

//XdkGc * xdk_screen_get_default_gc(XdkScreen * self);

//XdkVisual * xdk_screen_get_default_visual(XdkScreen * self);

struct _XdkDisplay * xdk_screen_get_display(XdkScreen * self);

glong xdk_screen_get_event_mask(XdkScreen * self);

struct _XdkWindow * xdk_screen_get_root_window(XdkScreen * self);

G_END_DECLS

#endif /* __XDK_SCREEN_H_ */
