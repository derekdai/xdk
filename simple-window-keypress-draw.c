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
	
	XSelectInput(display, window, ExposureMask | KeyPressMask);
	
	XMapWindow(display, window);
	
	XEvent event;
	while(TRUE) {
		XNextEvent(display, & event);
		
		switch(event.type) {
			case Expose:
				XFillRectangle(
					display, window,
					DefaultGC(display, screen),
					20, 20, 50, 50);
				XDrawString(
					display, window,
					DefaultGC(display, screen),
					300, 60,
					MESSAGE, sizeof(MESSAGE) - 1);
				break;
			case KeyPress:
				goto end;
		}
	}
	
end:
	g_message("Bye");
	
close_display:
	XCloseDisplay(display);
	
	return 0;
}
