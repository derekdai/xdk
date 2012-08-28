#ifndef __XDK_WINDOW_H_
#define __XDK_WINDOW_H_

#define XDK_WINDOW(o)			(XDK_CAST(o, XdkWindow, XDK_TYPE_WINDOW))

XdkWindow * xdk_window_new();

void xdk_window_set_peer(XdkWindow * self, Window peer);

Window xdk_window_get_peer(XdkWindow * self);

gboolean xdk_window_is_realized(XdkWindow * self);

void xdk_window_get_position(XdkWindow * self, int * x, int * y);

void xdk_window_set_position(XdkWindow * self, int x, int y);

void xdk_window_get_size(XdkWindow * self, int * width, int * height);

void xdk_window_set_size(XdkWindow * self, int width, int height);

#endif /* __XDK_WINDOW_H_ */
