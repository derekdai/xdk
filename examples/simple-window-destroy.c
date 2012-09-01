#include <glib.h>
#include <X11/Xlib.h>

gboolean quited = FALSE;

void on_delete(Display * display, Window window)
{
	XDestroyWindow(display, window);
	quited = TRUE;
	
	g_message("Window destroyed");
}

void on_destroy(Display * display, Window window)
{
	quited = TRUE;
	
	g_message("Exiting...");
}

int main()
{
	Display * display = XOpenDisplay(NULL);
	if(NULL == display) {
		g_error("Failed to initialize display");
	}
	
	Window root = DefaultRootWindow(display);
	if(None == root) {
		g_error("No root window found");
	}
	Window window = XCreateSimpleWindow(
		display, root,
		0, 0, 1280, 720,
		0, 0,
		0xffffffff);
	if(None == window) {
		g_error("Failed to create window");
	}
	XMapWindow(display, window);
	
	/* http://tronche.com/gui/x/icccm/sec-4.html#s-4.2.8.1 */
	/* http://stackoverflow.com/questions/1157364/intercept-wm-delete-window-on-x11 */
	/* http://john.nachtimwald.com/2009/11/08/sending-wm_delete_window-client-messages/ */
	/* http://stackoverflow.com/questions/10792361/how-do-i-gracefully-exit-an-x11-event-loop?rq=1 */
	
	Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", FALSE);
	XSetWMProtocols(display, window, & wm_delete_window, 1);
	
	XEvent event;
	while(! quited) {
		XNextEvent(display, & event);
		g_message("%d, %d, %d", ClientMessage, event.type);
		switch(event.type) {
		case ClientMessage:
			if(event.xclient.data.l[0] == wm_delete_window) {
				on_delete(event.xclient.display, event.xclient.window);
			}
			break;
		}
	}
	
	XCloseDisplay(display);
	
	return 0;
}
