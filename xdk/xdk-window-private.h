#ifndef __XDK_WINDOW_PRIVATE_H_
#define __XDK_WINDOW_PRIVATE_H_

#include <glib.h>
#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-base-private.h"

G_BEGIN_DECLS

typedef struct _XdkVisual XdkVisual;

struct _XdkWindow
{
	XdkBase parent;
	
	Window peer;
	
	XdkDisplay * display;
	
	XdkScreen * screen;
	
	XdkWindow * parent_window;
	
	gint x;
	
	gint y;
	
	guint width;
	
	guint height;
	
	XdkVisual * visual;
	
	gulong background_color;
	
	gboolean mapped : 1;
	
	gboolean visible : 1;
	
	gboolean own_peer : 1;
	
	gboolean destroyed : 1;
};

gboolean xdk_window_init(gpointer base);

G_END_DECLS

#endif /* __XDK_WINDOW_PRIVATE_H_ */
