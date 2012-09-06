#ifndef __XDK_WINDOW_H_
#define __XDK_WINDOW_H_

#include <glib-object.h>
#include <X11/Xlib.h>
#include "xdk-types.h"
#include "xdk-screen.h"

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
	
	/* virtual functions */
	void (* realize)(XdkWindow * self);
	
	void (* unrealize)(XdkWindow * self);
	
	/* delete event sent by window manager */
	void (* delete_event)(XdkWindow * self, XEvent * event);
	
	/* destroy emitted when unrealize by XdkWindow itself */
	void (* destroy)(XdkWindow * self);

	void (* configure_notify)(XdkWindow * self, XEvent * event);
	
	void (* map_notify)(XdkWindow * self, XEvent * event);
	
	void (* unmap_notify)(XdkWindow * self, XEvent * event);
	
	void (* gravity_notify)(XdkWindow * self, XEvent * event);
	
	void (* key_press)(XdkWindow * self, XEvent * event);
	
	void (* key_release)(XdkWindow * self, XEvent * event);
	
	void (* button_press)(XdkWindow * self, XEvent * event);
	
	void (* button_release)(XdkWindow * self, XEvent * event);
	
	void (* motion)(XdkWindow * self, XEvent * event);
	
	void (* enter)(XdkWindow * self, XEvent * event);
	
	void (* leave)(XdkWindow * self, XEvent * event);
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

XdkScreen * xdk_window_get_screen(XdkWindow * self);

void xdk_window_set_screen(XdkWindow * self, XdkScreen * screen);

void xdk_window_get_position(XdkWindow * self, int * x, int * y);

void xdk_window_set_position(XdkWindow * self, int x, int y);

void xdk_window_get_size(XdkWindow * self, guint * width, guint * height);

void xdk_window_set_size(XdkWindow * self, guint width, guint height);

void xdk_window_dispatch_event(XdkWindow * self, XEvent * event);

void xdk_window_set_attributes(
	XdkWindow * self,
	XdkWindowAttributeMask mask,
	XSetWindowAttributes * attributes);

void xdk_window_get_attributes(
	XdkWindow * self,
	XSetWindowAttributes * attributes);

void xdk_window_set_gravity(XdkWindow * self, XdkGravity gravity);

XdkGravity xdk_window_get_gravity(XdkWindow * self);

void xdk_window_raise(XdkWindow * self);

void xdk_window_lower(XdkWindow * self);

void xdk_window_set_borser_width(XdkWindow * self, guint width);

guint xdk_window_get_borser_width(XdkWindow * self);

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

void xdk_window_event_mask_set(XdkWindow * self, XdkEventMask event_mask);

void xdk_window_event_mask_add(XdkWindow * self, XdkEventMask event_mask);

void xdk_window_event_mask_remove(XdkWindow * self, XdkEventMask event_mask);

gulong xdk_window_event_mask_get(XdkWindow * self);

void xdk_window_set_wm_protocols(XdkWindow * self, Atom * protocols, gint n_protocols);

Atom * xdk_window_get_wm_protocols(XdkWindow * self, gint * n_protocols);

void xdk_window_add_child(XdkWindow * self, XdkWindow * child);

void xdk_window_remove_child(XdkWindow * self, XdkWindow * child);

gboolean xdk_window_contains_child(XdkWindow * self, XdkWindow * child);

GList * xdk_window_list_children(XdkWindow * self);

void xdk_window_show(XdkWindow * self);

void xdk_window_all(XdkWindow * self);

void xdk_window_hide(XdkWindow * self);

void xdk_window_select_input(XdkWindow * self, XdkEventMask event_mask);

G_END_DECLS

#endif /* __XDK_WINDOW_H_ */
