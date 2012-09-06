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

typedef int (* XdkErrorHandler)(XdkDisplay * display, XErrorEvent * error);

typedef gboolean (* XdkEventFilter)(XdkDisplay * display, XEvent * event, gpointer user_data);

struct _XdkDisplayClass
{
	GObjectClass base;
};

struct _XdkDisplay
{
	GObject base;
	
	XdkDisplayPrivate * priv;
};

/**
 * keys to get atom from xdk_display_atom_get()
 */
extern GQuark XDK_ATOM_WM_DELETE_WINDOW;

GType xdk_display_get_type();

gboolean xdk_display_init_once();

XdkDisplay * xdk_display_get_default();

Display * xdk_display_get_peer(XdkDisplay * self);

const char * xdk_display_get_vendor(XdkDisplay * self);

gint xdk_display_get_release(XdkDisplay * self);

const gchar * xdk_display_get_name(XdkDisplay * self);

void xdk_display_flush(XdkDisplay * self);

void xdk_flush();

Atom xdk_atom_from_name(
	const char * atom_name,
	gboolean only_if_exists);
	
gchar * xdk_atom_to_name(Atom atom);

Atom xdk_display_atom_from_name(
	XdkDisplay * self,
	const char * atom_name,
	gboolean only_if_exists);
	
gchar * xdk_display_atom_to_name(XdkDisplay * self, Atom atom);

void xdk_display_add_window(XdkDisplay * self, XdkWindow * window);

XdkWindow * xdk_display_lookup_window(XdkDisplay * self, Window xwindow);

void xdk_display_remove_window(XdkDisplay * self, XdkWindow * window);

int xdk_display_get_connection_number(XdkDisplay * self);

void xdk_display_add_watch(XdkDisplay * self);

void xdk_display_remove_watch(XdkDisplay * self);

GSource * xdk_display_watch_source_new(XdkDisplay * self);

void xdk_display_add_event_filter(
	XdkDisplay * self,
	XdkEventFilter filter,
	gpointer user_data);

void xdk_display_remove_event_filter(
	XdkDisplay * self,
	XdkEventFilter filter,
	gpointer user_data);

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self);

gint xdk_display_get_n_screens(XdkDisplay * self);

GList * xdk_display_list_screens(XdkDisplay * display);

gint xdk_display_get_n_windows(XdkDisplay * self);

GList * xdk_display_list_windows(XdkDisplay * display);

GList * xdk_display_list_xwindows(XdkDisplay * display);

XdkScreen * xdk_display_lookup_screen(XdkDisplay * display, Screen * screen);

XdkScreen * xdk_get_default_screen();

XdkWindow * xdk_get_default_root_window();

void xdk_trap_error();

gint xdk_untrap_error();

G_END_DECLS

#endif /* __XDK_DISPLAY_H_ */
