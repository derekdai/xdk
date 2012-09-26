#ifndef __XDK_DISPLAY_H_
#define __XDK_DISPLAY_H_

#include <glib-object.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
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

/**
 * root window properties
 * @see http://standards.freedesktop.org/wm-spec/wm-spec-1.5.html#id2533796
 */
extern GQuark XDK_ATOM_NET_SUPPORTED;
extern GQuark XDK_ATOM_NET_CLIENT_LIST;
extern GQuark XDK_ATOM_NET_NUMBER_OF_DESKTOPS;
extern GQuark XDK_ATOM_NET_DESKTOP_GEOMETRY;
extern GQuark XDK_ATOM_NET_DESKTOP_VIEWPORT;
extern GQuark XDK_ATOM_NET_CURRENT_DESKTOP;
extern GQuark XDK_ATOM_NET_DESKTOP_NAMES;
extern GQuark XDK_ATOM_NET_ACTIVE_WINDOW;
extern GQuark XDK_ATOM_NET_WORKAREA;
extern GQuark XDK_ATOM_NET_SUPPORTING_WM_CHECK;
extern GQuark XDK_ATOM_NET_VIRTUAL_ROOTS;
extern GQuark XDK_ATOM_NET_DESKTOP_LAYOUT;
extern GQuark XDK_ATOM_NET_SHOWING_DESKTOP;

/**
 * other root window messages
 * @see http://standards.freedesktop.org/wm-spec/wm-spec-1.5.html#id2577565
 */
extern GQuark XDK_ATOM_NET_WINDOW;
extern GQuark XDK_ATOM_NET_MOVERESIZE_WINDOW;
extern GQuark XDK_ATOM_NET_WM_MOVERESIZE;
extern GQuark XDK_ATOM_NET_RESTACK_WINDOW;
extern GQuark XDK_ATOM_NET_REQUEST_FRAME_EXTENTS;

/**
 * application window properties
 * @see http://standards.freedesktop.org/wm-spec/wm-spec-1.5.html#id2577833
 */
extern GQuark XDK_ATOM_NET_WM_NAME;
extern GQuark XDK_ATOM_NET_WM_VISIBLE_NAME;
extern GQuark XDK_ATOM_NET_WM_ICON_NAME;
extern GQuark XDK_ATOM_NET_WM_VISIBLE_ICON_NAME;
extern GQuark XDK_ATOM_NET_WM_DESKTOP;
extern GQuark XDK_ATOM_NET_WM_WINDOW_TYPE;
extern GQuark XDK_ATOM_NET_WM_STATE;
extern GQuark XDK_ATOM_NET_WM_ALLOWED_ACTIONS;
extern GQuark XDK_ATOM_NET_WM_STRUT;
extern GQuark XDK_ATOM_NET_WM_STRUT_PARTIAL;
extern GQuark XDK_ATOM_NET_WM_ICON_GEOMETRY;
extern GQuark XDK_ATOM_NET_WM_ICON;
extern GQuark XDK_ATOM_NET_WM_PID;
extern GQuark XDK_ATOM_NET_WM_HANDLED_ICONS;
extern GQuark XDK_ATOM_NET_WM_USER_TIME;
extern GQuark XDK_ATOM_NET_WM_USER_TIME_WINDOW;
extern GQuark XDK_ATOM_NET_FRAME_EXTENTS;
extern GQuark XDK_ATOM_NET_WM_OPAQUE_REGION;

/**
 * window manager protocols
 * @see http://standards.freedesktop.org/wm-spec/wm-spec-1.5.html#id2578884
 */
extern GQuark XDK_ATOM_NET_WM_PING;
extern GQuark XDK_ATOM_NET_WM_SYNC_REQUEST;
extern GQuark XDK_ATOM_NET_WM_FULLSCREEN_MONITORS;

/**
 * 
 * @see http://standards.freedesktop.org/wm-spec/wm-spec-1.5.html#id2578152
 */
extern GQuark XDK_ATOM_NET_WM_STATE_MODAL;
extern GQuark XDK_ATOM_NET_WM_STATE_STICKY;
extern GQuark XDK_ATOM_NET_WM_STATE_MAXIMIZED_VERT;
extern GQuark XDK_ATOM_NET_WM_STATE_MAXIMIZED_HORZ;
extern GQuark XDK_ATOM_NET_WM_STATE_SHADED;
extern GQuark XDK_ATOM_NET_WM_STATE_SKIP_TASKBAR;
extern GQuark XDK_ATOM_NET_WM_STATE_SKIP_PAGER;
extern GQuark XDK_ATOM_NET_WM_STATE_HIDDEN;
extern GQuark XDK_ATOM_NET_WM_STATE_FULLSCREEN;
extern GQuark XDK_ATOM_NET_WM_STATE_ABOVE;
extern GQuark XDK_ATOM_NET_WM_STATE_BELOW;
extern GQuark XDK_ATOM_NET_WM_STATE_DEMANDS_ATTENTION;
extern GQuark XDK_ATOM_NET_WM_STATE_FOCUSED;

/**
 * other properties
 * @see http://standards.freedesktop.org/wm-spec/wm-spec-1.5.html#id2579111
 */
extern GQuark XDK_ATOM_NET_WM_FULLPLACEMENT;

/**
 * compositing manager
 * @see http://standards.freedesktop.org/wm-spec/wm-spec-1.5.html#id2579173
 */
extern GQuark XDK_ATOM_NET_WM_CM_S0;
extern GQuark XDK_ATOM_NET_WM_CM_S1;
extern GQuark XDK_ATOM_NET_WM_CM_S2;
extern GQuark XDK_ATOM_NET_WM_CM_S3;
extern GQuark XDK_ATOM_WM_TRANSIENT_FOR;

/**
 * ICCCM client properties
 * @see http://tronche.com/gui/x/icccm/sec-4.html#s-4.1.2
 */
extern GQuark XDK_ATOM_WM_NAME;
extern GQuark XDK_ATOM_WM_ICON_NAME;
extern GQuark XDK_ATOM_WM_NORMAL_HINTS;
extern GQuark XDK_ATOM_WM_HINTS;
extern GQuark XDK_ATOM_WM_CLASS;
extern GQuark XDK_ATOM_WM_TRANSIENT_FOR;
extern GQuark XDK_ATOM_WM_PROTOCOLS;
extern GQuark XDK_ATOM_WM_COLORMAP_WINDOWS;
extern GQuark XDK_ATOM_WM_CLIENT_MACHINE;

/**
 * ICCCM window manager properties
 * @see http://tronche.com/gui/x/icccm/sec-4.html#s-4.1.3
 */
extern GQuark XDK_ATOM_WM_STATE;
extern GQuark XDK_ATOM_WM_ICON_SIZE;

GType xdk_display_get_type();

XdkDisplay * xdk_display_get_default();

gboolean xdk_display_open(XdkDisplay * self);

Display * xdk_display_get_peer(XdkDisplay * self);

Display * xdk_get_default_xdisplay();

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

gboolean xdk_display_handle_event(XdkDisplay * self, XEvent * event);

void xdk_display_add_event_filter(
	XdkDisplay * self,
	XdkEventFilter filter,
	gpointer user_data);

void xdk_display_remove_event_filter(
	XdkDisplay * self,
	XdkEventFilter filter,
	gpointer user_data);

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self);

XdkScreen * xdk_display_get_screen(XdkDisplay * self, gint screen_number);

gint xdk_display_get_n_screens(XdkDisplay * self);

GList * xdk_display_list_screens(XdkDisplay * self);

gint xdk_display_get_n_windows(XdkDisplay * self);

GList * xdk_display_list_windows(XdkDisplay * self);

GList * xdk_display_list_xwindows(XdkDisplay * self);

XdkScreen * xdk_display_lookup_screen(XdkDisplay * self, Screen * screen);

XdkVisual * xdk_display_lookup_visual(XdkDisplay * self, Visual * visual);

XdkVisual * xdk_display_lookup_visual_by_id(XdkDisplay * self, VisualID id);

int xdk_display_grab_server(XdkDisplay * self);

int xdk_display_ungrab_server(XdkDisplay * self);

gboolean xdk_display_has_composite_extension(XdkDisplay * self);

gboolean xdk_display_has_damage_extension(XdkDisplay * self);

Cursor xdk_display_create_font_cursor(XdkDisplay * self, guint shape);

void xdk_display_free_cursor(XdkDisplay * self, Cursor cursor);

XdkScreen * xdk_get_default_screen();

XdkWindow * xdk_get_default_root_window();

void xdk_trap_error();

gint xdk_untrap_error();

G_END_DECLS

#endif /* __XDK_DISPLAY_H_ */
