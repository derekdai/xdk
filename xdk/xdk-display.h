#ifndef __XDK_DISPLAY_H_
#define __XDK_DISPLAY_H_

#define XDK_DISPLAY(o)			(XDK_CAST(o, XdkDisplay, XDK_TYPE_DISPLAY))

XdkDisplay * xdk_display_get_default();

Display * xdk_display_get_peer(XdkDisplay * self);

const char * xdk_display_get_vendor(XdkDisplay * self);

gint xdk_display_get_release(XdkDisplay * self);

const gchar * xdk_display_get_name(XdkDisplay * self);

gint xdk_display_get_n_screens(XdkDisplay * self);

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self);

void xdk_display_flush(XdkDisplay * self);

XdkWindow * xdk_get_default_root_window();

#endif /* __XDK_DISPLAY_H_ */
