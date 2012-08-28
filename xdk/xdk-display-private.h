#ifndef __XDK_DISPLAY_PRIVATE_H_
#define __XDK_DISPLAY_PRIVATE_H_

struct _XdkDisplay
{
	XdkBase parent;
	
	gchar * name;
	
	Display * peer;
	
	gint n_screens;
	
	gint default_screen;
	
	XdkScreen ** screens;
};

gboolean xdk_display_init(gpointer base);

gboolean xdk_display_init_once();

void xdk_display_destroy(gpointer base);

#endif /* __XDK_DISPLAY_PRIVATE_H_ */
