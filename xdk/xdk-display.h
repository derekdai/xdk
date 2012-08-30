#ifndef __XDK_DISPLAY_H_
#define __XDK_DISPLAY_H_

#include <glib.h>
#include <X11/Xlib.h>
#include "xdk-screen.h"
#include "xdk-window.h"

G_BEGIN_DECLS

#define XDK_DISPLAY(o)			(XDK_CAST(o, XdkDisplay, XDK_TYPE_DISPLAY))

typedef struct _XdkDisplay XdkDisplay;

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
