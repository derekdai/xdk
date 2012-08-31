#ifndef __XDK_WINDOW_H_
#define __XDK_WINDOW_H_

#include <glib-object.h>
#include <X11/Xlib.h>
#include "xdk-types.h"

G_BEGIN_DECLS

#define XDK_TYPE_WINDOW (xdk_window_get_type())
#define XDK_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), XDK_TYPE_WINDOW, XdkWindow))
#define XDK_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XDK_TYPE_WINDOW, XdkWindowClass))
#define IS_XDK_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XDK_TYPE_WINDOW))
#define IS_XDK_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XDK_TYPE_WINDOW))
#define XDK_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), XDK_TYPE_WINDOW, XdkWindowClass))

typedef struct _XdkWindowClass XdkWindowClass;

typedef struct _XdkWindow XdkWindow;

typedef struct _XdkWindowPrivate XdkWindowPrivate;

struct _XdkWindowClass
{
	GObjectClass base;
	
	void (* realize)(XdkWindow * self);
};

struct _XdkWindow
{
	GObject base;
	
	XdkWindowPrivate * priv;
};

GType xdk_window_get_type();

XdkWindow * xdk_window_new();

void xdk_window_set_foreign_peer(XdkWindow * self, Window peer);

void xdk_window_take_peer(XdkWindow * self, Window peer);

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

void xdk_window_set_attributes(
	XdkWindow * self,
	XdkWindowAttributeMask mask,
	XSetWindowAttributes * attributes);

void xdk_window_get_attributes(
	XdkWindow * self,
	XSetWindowAttributes * attributes);

void xdk_window_set_gravity(XdkWindow * self, XdkGravity gravity);

XdkGravity xdk_window_get_gravity(XdkWindow self);

Atom * xdk_window_list_properties(XdkWindow * self, int * n_props);

Window xdk_window_create_window(
	XdkWindow * self,
	XdkWindow * parent,
	XdkWindowClasses window_class,
	XdkWindowAttributeMask attribute_mask,
	XSetWindowAttributes *attributes);

void xdk_window_set_parent(XdkWindow * self, XdkWindow * parent);

XdkWindow * xdk_window_get_parent(XdkWindow * self);

void xdk_window_set_background_color(XdkWindow * self, gulong background_color);

gulong xdk_window_get_background_color(XdkWindow * self);

G_END_DECLS

#endif /* __XDK_WINDOW_H_ */
