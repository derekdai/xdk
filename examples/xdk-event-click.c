#include <xdk/xdk.h>

int main()
{
	xdk_init(NULL, NULL);
	
	XdkWindow * win = xdk_window_new();
	xdk_window_show(win);
	
	XEvent event;
	xdk_next_event(& event);
	
	return 0;
}
