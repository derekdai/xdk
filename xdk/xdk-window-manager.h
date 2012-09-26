#ifndef __XDK_WINDOW_MANAGER_H_
#define __XDK_WINDOW_MANAGER_H_

#include <glib-object.h>
#include "xdk-screen.h"

G_BEGIN_DECLS

#define XDK_WINDOW_MANAGER_ERROR (xdk_window_manager_error())
#define XDK_TYPE_WINDOW_MANAGER (xdk_window_manager_get_type())
#define XDK_WINDOW_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), XDK_TYPE_WINDOW_MANAGER, XdkWindowManager))
#define XDK_WINDOW_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XDK_TYPE_WINDOW_MANAGER, XdkWindowManagerClass))
#define IS_XDK_WINDOW_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XDK_TYPE_WINDOW_MANAGER))
#define IS_XDK_WINDOW_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XDK_TYPE_WINDOW_MANAGER))
#define XDK_WINDOW_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), XDK_TYPE_WINDOW_MANAGER, XdkWindowManagerClass))

typedef struct _XdkWindowManager XdkWindowManager;

typedef struct _XdkWindowManagerClass XdkWindowManagerClass;

typedef struct _XdkWindowManagerPrivate XdkWindowManagerPrivate;

struct _XdkWindowManagerClass
{
	GObjectClass base;
	
	void (* screen_added)(XdkWindowManager * self, XdkScreen * screen);

	void (* screen_removed)(XdkWindowManager * self, XdkScreen * screen);
};

struct _XdkWindowManager
{
	GObject base;
	
	XdkWindowManagerPrivate * priv;
};

enum
{
	XDK_WINDOW_MANAGER_ERROR_WM_EXISTS = 1,
	XDK_WINDOW_MANAGER_ERROR_CM_EXISTS,
};

GType xdk_window_manager_get_type();

GQuark xdk_window_manager_error();

/**
 * Create a window manager for specified screen.
 * 
 * @screen: if NULL, default screen will be used
 */
XdkWindowManager * xdk_window_manager_new();

void xdk_window_manager_set_default_cursor(
	XdkWindowManager * self,
	Cursor cursor);

Cursor xdk_window_manager_get_default_cursor(XdkWindowManager * self);

void xdk_window_manager_unset_default_cursor(XdkWindowManager * self);

/**
 * @screen: if NULL, add default screen of defautl display
 */
gboolean xdk_window_manager_add_screen(
	XdkWindowManager * self,
	XdkScreen * screen,
	GError ** error);

/**
 * @display: if NULL, add all screens of default display
 */
gboolean xdk_window_manager_add_all_screens(
	XdkWindowManager * self,
	XdkDisplay * display,
	GError ** error);

gboolean xdk_window_manager_has_screen(
	XdkWindowManager * self,
	XdkScreen * screen);

void xdk_window_manager_remove_screen(
	XdkWindowManager * self,
	XdkScreen * screen);

void xdk_window_manager_remove_all_screens(XdkWindowManager * self);

GList * xdk_window_manager_list_screens(XdkWindowManager * self);

gint xdk_window_manager_n_screens(XdkWindowManager * self);

void xdk_window_manager_set_background_color(
	XdkWindowManager * self,
	gulong background_color);

gulong xdk_window_manager_get_background_color(XdkWindowManager * self);

G_END_DECLS

#endif /* __XDK_WINDOW_MANAGER_H_ */
