#include <xdk/xdk.h>

int main(gint argc, gchar * args[])
{
	xdk_init(& argc, & args);
	
	XdkDisplay * display = xdk_display_get_default();
	XdkScreen * screen = xdk_display_get_default_screen(display);

	GError * error = NULL;
	XdkWindowManager * wm;
	if(! (wm = xdk_window_manager_new(screen, & error))) {
		if(error) {
			g_error("%s", error->message);
			g_error_free(error);
		}
	}
	
	xdk_main();
	
	return 0;
}
