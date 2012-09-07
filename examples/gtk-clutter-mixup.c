#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <xdk/xdk.h>

GdkFilterReturn on_event(GdkXEvent * xevent, GdkEvent * event, GtkWidget * widget)
{
	xdk_util_event_dump((XEvent *) xevent);
	
	if(CLUTTER_X11_FILTER_CONTINUE == clutter_x11_handle_event((XEvent *) xevent)) {
		return GDK_FILTER_CONTINUE;
	}

	return GDK_FILTER_REMOVE;
}

void allocation_changed(ClutterActor * stage, ClutterActorBox * box, ClutterAllocationFlags flags, ClutterActor * rect)
{
	float width = box->x2 - box->x1, height = box->y2 - box->y1;
	clutter_actor_set_position(rect, width / 4, height / 4);
	clutter_actor_set_size(rect, width - (width / 2), height - (height / 2));
}

void on_size_allocate(GtkWidget * widget, GdkRectangle * rect, ClutterActor * stage)
{
	clutter_actor_set_position(stage, rect->x, rect->y);
	clutter_actor_set_size(stage, rect->width, rect->height);
}

void on_draw(GtkWidget * widget, cairo_t * cr, ClutterActor * stage)
{
	clutter_actor_queue_redraw(stage);
	
	return FALSE;
}

void restart_animate(ClutterActor * actor)
{
	g_return_if_fail(CLUTTER_IS_ACTOR(actor));
	
	g_object_set(actor, "rotation-angle-z", 0.0, NULL);
	clutter_actor_animate(actor, CLUTTER_LINEAR, 5000,
		"rotation-angle-z", 360.0,
		"signal-swapped-after::completed", restart_animate, actor,
		NULL);
}

int main(int argc, char * args[])
{
	gtk_init(& argc, & args);
	clutter_x11_disable_event_retrieval();
	clutter_x11_set_display(gdk_x11_get_default_xdisplay());
	clutter_init(& argc, & args);
	
	GtkWidget * win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(win, "delete-event", G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	GtkWidget * da = gtk_drawing_area_new();
	gdk_window_add_filter(gtk_widget_get_window(da), (GdkFilterFunc) on_event, da);
	gtk_container_add(GTK_CONTAINER(win), da);
	gtk_widget_realize(da);
	
	ClutterActor * stage = clutter_stage_new();
	clutter_x11_set_stage_foreign(CLUTTER_STAGE(stage), gdk_x11_window_get_xid(gtk_widget_get_window(da)));
	clutter_actor_realize(stage);
	g_object_set_data(G_OBJECT(da), "stage", stage);
	g_signal_connect(da, "size-allocate", G_CALLBACK(on_size_allocate), stage);
	g_signal_connect_swapped(da, "unrealize", G_CALLBACK(clutter_actor_unrealize), stage);
	g_signal_connect_swapped(da, "map", G_CALLBACK(clutter_actor_map), stage);
	g_signal_connect_swapped(da, "unmap", G_CALLBACK(clutter_actor_unmap), stage);
	g_signal_connect_swapped(da, "show", G_CALLBACK(clutter_actor_show), stage);
	g_signal_connect_swapped(da, "hide", G_CALLBACK(clutter_actor_hide), stage);
	g_signal_connect(da, "draw", G_CALLBACK(on_draw), stage);

	ClutterColor c = { 0xff, 0, 0, 0x7f };
	ClutterActor * rect = clutter_rectangle_new_with_color(& c);
	clutter_actor_add_child(stage, rect);
	g_signal_connect(stage, "allocation-changed", G_CALLBACK(allocation_changed), rect);
	restart_animate(rect);

	clutter_actor_show(rect);
	clutter_actor_show(stage);
	gtk_widget_show_all(win);
	
	gtk_main();
}
