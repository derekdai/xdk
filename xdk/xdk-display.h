#ifndef __XDK_DISPLAY_H_
#define __XDK_DISPLAY_H_

#include <glib-object.h>
#include <X11/Xlib.h>
#include "xdk-screen.h"
#include "xdk-window.h"

G_BEGIN_DECLS

#define XDK_TYPE_DISPLAY (xdk_display_get_type())
#define XDK_DISPLAY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), XDK_TYPE_DISPLAY, XdkDisplay))
#define XDK_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XDK_TYPE_DISPLAY, XdkDisplayClass))
#define IS_XDK_DISPLAY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XDK_TYPE_DISPLAY))
#define IS_XDK_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XDK_TYPE_DISPLAY))
#define XDK_DISPLAY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), XDK_TYPE_DISPLAY, XdkDisplayClass))

typedef struct _XdkDisplayClass XdkDisplayClass;

typedef struct _XdkDisplay XdkDisplay;

typedef struct _XdkDisplayPrivate XdkDisplayPrivate;

struct _XdkDisplayClass
{
	GObjectClass base;
};

struct _XdkDisplay
{
	GObject base;
	
	XdkDisplayPrivate * priv;
};

GType xdk_display_get_type();

XdkDisplay * xdk_display_new();

gboolean xdk_display_init_once();

XdkDisplay * xdk_display_get_default();

Display * xdk_display_get_peer(XdkDisplay * self);

const char * xdk_display_get_vendor(XdkDisplay * self);

gint xdk_display_get_release(XdkDisplay * self);

const gchar * xdk_display_get_name(XdkDisplay * self);

gint xdk_display_get_n_screens(XdkDisplay * self);

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self);

void xdk_display_flush(XdkDisplay * self);

Atom xdk_atom_from_name(
	const char * atom_name,
	gboolean only_if_exists);
	
gchar * xdk_atom_to_name(Atom atom);

void xdk_display_add_window(XdkDisplay * self, XdkWindow * window);

XdkWindow * xdk_display_lookup_window(XdkDisplay * self, Window xwindow);

void xdk_display_remove_window(XdkDisplay * self, XdkWindow * window);

gboolean xdk_display_next_event(XdkDisplay * self);

void xdk_display_dispatch_event(XdkDisplay * self);

XdkScreen * xdk_get_default_screen();

XdkWindow * xdk_get_default_root_window();

G_END_DECLS

#endif /* __XDK_DISPLAY_H_ */
