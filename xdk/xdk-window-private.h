#ifndef __XDK_WINDOW_PRIVATE_H_
#define __XDK_WINDOW_PRIVATE_H_

struct _XdkWindow
{
	XdkBase parent;
	
	XdkDisplay * display;
	
	XdkScreen * screen;
	
	XdkWindow * parent_window;
	
	Window peer;
	
	gint x;
	
	gint y;
	
	guint width;
	
	guint height;
	
	gint depth;
	
	XdkVisual * visual;
	
	gulong background_color;
	
	gboolean mapped : 1;
	
	gboolean visible : 1;
	
	gboolean own_peer : 1;
};

gboolean xdk_window_init(gpointer base);

#endif /* __XDK_WINDOW_PRIVATE_H_ */
