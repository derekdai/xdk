#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>

ClutterActor * stage1;

ClutterActor * actor;

gboolean on_timeout(gpointer user_data)
{
	static gulong color = 0x0000ff7f;

	color <<= 1;
	if(! color) {
		color = 0x0000ff7f;
	}
	
	ClutterColor cc;
	clutter_color_from_pixel(& cc, color);
	
	clutter_actor_set_background_color(stage1, & cc);
	clutter_x11_texture_pixmap_update_area(actor, 0, 0, 720, 480);
	
	return TRUE;
}

void on_update_area(ClutterX11TexturePixmap *texture, gint x, gint y, gint width, gint height)
{
	g_message("update area %d %d %d %d", x, y, width, height);
}

void on_queue_damage_redraw(ClutterX11TexturePixmap *texture, gint x, gint y, gint width, gint height)
{
	g_message("queue damage redraw %d %d %d %d", x, y, width, height);
}

int main()
{
	clutter_init(NULL, NULL);
	gtk_init(NULL, NULL);
	
	if(! clutter_x11_has_composite_extension()) {
		g_error("XComposite is needed");
	}
	
	stage1 = clutter_stage_new();
	clutter_actor_realize(stage1);
	clutter_actor_set_position(stage1, 400, 250);
	clutter_actor_set_size(stage1, 800, 600);
	clutter_actor_set_background_color(stage1, clutter_color_get_static(CLUTTER_COLOR_RED));
	
	ClutterActor * stage2 = clutter_stage_new();
	clutter_actor_set_size(stage2, 720, 480);
	clutter_actor_set_background_color(stage2, clutter_color_get_static(CLUTTER_COLOR_BLUE));
	clutter_stage_set_user_resizable(stage2, TRUE);
	
	actor = clutter_x11_texture_pixmap_new();
	g_signal_connect(actor, "update-area", on_update_area, NULL);
	g_signal_connect(actor, "queue-damage-redraw", on_queue_damage_redraw, NULL);
	clutter_actor_set_size(actor, 720, 480);
	clutter_actor_add_child(stage2, actor);

	// comment out this line to disable sync between stage1 and stage2
	clutter_x11_texture_pixmap_set_window(actor, clutter_x11_get_stage_window(stage1), FALSE);
	
	g_timeout_add(100, on_timeout, NULL);

	clutter_actor_show(stage1);
	clutter_actor_show(stage2);
	
	clutter_main();
	
	return 0;
}
