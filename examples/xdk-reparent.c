#define G_LOG_DOMAIN "XdkParentChild"

#include <xdk/xdk.h>
#include <stdlib.h>

XdkWindow * parent1 = NULL;
XdkWindow * parent2 = NULL;

gboolean on_timeout(XdkWindow * child)
{
	XdkWindow * parent = xdk_window_get_parent(child);
	if(parent == parent1) {
		parent = parent2;
		g_message("Reparent to parent2");
	}
	else {
		parent = parent1;
		g_message("Reparent to parent1");
	}
	
	xdk_window_add_child(parent, child);
	
	return TRUE;
}

int main(gint argc, gchar * args[])
{
	xdk_init(& argc, & args);
	
	/*
	parent1 = xdk_window_new();
	xdk_window_set_size(parent1, 320, 240);
	g_signal_connect(parent1, "destroy", G_CALLBACK(xdk_main_quit), NULL);
	
	parent2 = xdk_window_new();
	xdk_window_set_size(parent2, 320, 240);
	g_signal_connect(parent2, "destroy", G_CALLBACK(xdk_main_quit), NULL);
	
	XdkWindow * child = xdk_window_new();
	xdk_window_set_background_color(child, 0x00ff00);
	xdk_window_set_position(child, 50, 50);
	xdk_window_set_size(child, 50, 50);
	xdk_window_add_child(parent1, child);
	//g_object_unref(child);
	
	xdk_window_show_all(parent1);
	xdk_window_show_all(parent2);
	*/
	
	/*XReparentWindow(
		xdk_display_get_peer(xdk_display_get_default()),
		xdk_window_get_peer(child),
		xdk_window_get_peer(parent2),
		0, 0);*/
	
	//g_timeout_add_seconds(3, (GSourceFunc) on_timeout, child);
	
	Window root, parent;
	Window * children;
	guint n_children;
	XQueryTree(
		xdk_display_get_peer(xdk_display_get_default()),
		xdk_window_get_peer(xdk_get_default_root_window()),
		& root,
		& parent,
		& children,
		& n_children);
	g_message("%d windows found", n_children);
	
	exit(0);
	
	xdk_main();
	
	g_object_unref(parent2);
	g_object_unref(parent1);
	
	return 0;
}

