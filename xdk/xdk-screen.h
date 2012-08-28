#ifndef __XDK_DISPLAY_H_
#define __XDK_DISPLAY_H_

#define XDK_SCREEN(o)			(XDK_CAST(o, XdkScreen, XDK_TYPE_SCREEN))

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

#endif /* __XDK_DISPLAY_H_ */
