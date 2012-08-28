#include <X11/Xlib.h>
#include <glib.h>

#define MESSAGE ("Hello World")

int main(int argc, char * argv)
{
	Display * display = XOpenDisplay(NULL);
	if(! display) {
		g_error("Failed to open display");
	}
	
	int screen = DefaultScreen(display);
	Window window = XCreateSimpleWindow(
		display,
		RootWindow(display, screen),
		0, 0, 1280, 720, 5, /* border width */
		BlackPixel(display, screen), WhitePixel(display, screen));
	if(None == window) {
		g_error("Failed to create window");
	}
	
	XMapWindow(display, window);
	XFlush(display);
	
	usleep(3 * 1000 * 1000);
	
	g_message("Bye");
	
close_display:
	XCloseDisplay(display);
	
	return 0;
}

