#ifndef __XDK_SCREEN_H_
#define __XDK_SCREEN_H_

#include <glib.h>
#include <X11/Xlib.h>

G_BEGIN_DECLS

#define XDK_SCREEN(o)			(XDK_CAST(o, XdkScreen, XDK_TYPE_SCREEN))

typedef struct _XdkScreen XdkScreen;

struct _XdkDisplay;

struct _XdkWindow;

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
