#ifndef __XDK_WINDOW_H_
#define __XDK_WINDOW_H_

#include <glib.h>
#include <X11/Xlib.h>

G_BEGIN_DECLS

#define XDK_WINDOW(o)			(XDK_CAST(o, XdkWindow, XDK_TYPE_WINDOW))

typedef struct _XdkWindow XdkWindow;

XdkWindow * xdk_window_new();

void xdk_window_set_foreign_peer(XdkWindow * self, Window peer);

Window xdk_window_get_peer(XdkWindow * self);

void xdk_window_realize(XdkWindow * self);

gboolean xdk_window_is_realized(XdkWindow * self);

void xdk_window_unrealize(XdkWindow * self);

void xdk_window_map(XdkWindow * self);

gboolean xdk_window_is_mapped(XdkWindow * self);

void xdk_window_unmap(XdkWindow * self);

void xdk_window_destroy(XdkWindow * self);

void xdk_window_get_position(XdkWindow * self, int * x, int * y);

void xdk_window_set_position(XdkWindow * self, int x, int y);

void xdk_window_get_size(XdkWindow * self, int * width, int * height);

void xdk_window_set_size(XdkWindow * self, int width, int height);

void xdk_window_handle_event(XdkWindow * self, XEvent * event);

Atom * xdk_window_list_properties(XdkWindow * self, int * n_props);

G_END_DECLS

#endif /* __XDK_WINDOW_H_ */
