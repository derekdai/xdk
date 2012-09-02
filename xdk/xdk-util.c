#include "xdk-util.h"

static const char * event_names[] = {
	"Unknown",
	"Unknown",
	"XDK_EVENT_KEY_PRESS (KeyPress)",
	"XDK_EVENT_KEY_RELEASE (KeyRelease)",
	"XDK_EVENT_BUTTON_PRESS (ButtonPress)",
	"XDK_EVENT_BUTTON_RELEASE (ButtonRelease)",
	"XDK_EVENT_MOTION (MotionNotify)",
	"XDK_EVENT_ENTER (EnterNotify)",
	"XDK_EVENT_LEAVE (LeaveNotify)",
	"XDK_EVENT_FOCUS_IN (FocusIn)",
	"XDK_EVENT_FOCUS_OUT (FocusOut)",
	"XDK_EVENT_KEYMAP (KeymapNotify)",
	"XDK_EVENT_EXPOSE (Expose)",
	"XDK_EVENT_GRAPHICS_EXPOSE (GraphicsExpose)",
	"XDK_EVENT_NO_EXPOSE (NoExpose)",
	"XDK_EVENT_VISIBILITY (VisibilityNotify)",
	"XDK_EVENT_CREATE (CreateNotify)",
	"XDK_EVENT_DESTROY (DestroyNotify)",
	"XDK_EVENT_UNMAP (UnmapNotify)",
	"XDK_EVENT_MAP (MapNotify)",
	"XDK_EVENT_MAP_REQUEST (MapRequest)",
	"XDK_EVENT_REPARENT (ReparentNotify)",
	"XDK_EVENT_CONFIGURE (ConfigureNotify)",
	"XDK_EVENT_CONFIGURE_REQUEST (ConfigureRequest)",
	"XDK_EVENT_GRAVITY (GravityNotify)",
	"XDK_EVENT_RESIZE_REQUEST (ResizeRequest)",
	"XDK_EVENT_CIRCULATE (CirculateNotify)",
	"XDK_EVENT_CIRCULATE_REQUEST = CirculateRequest)",
	"XDK_EVENT_PROPERTY (PropertyNotify)",
	"XDK_EVENT_SELECTION_CLEAR (SelectionClear)",
	"XDK_EVENT_SELECTION_REQUEST (SelectionRequest)",
	"XDK_EVENT_SELECTION (SelectionNotify)",
	"XDK_EVENT_COLORMAP (ColormapNotify)",
	"XDK_EVENT_CLIENT_MESSAGE (ClientMessage)",
	"XDK_EVENT_MAPPING (MappingNotify)",
	"XDK_EVENT_GENERIC (GenericEvent)",
	"XDK_EVENT_LAST (LASTEvent)",
};

void xdk_util_event_dump(XEvent * event)
{
	gchar * desc = xdk_util_event_to_string(event);
	if(desc) {
		g_printerr("%s\n", desc);
		g_free(desc);
	}
}

gchar * xdk_util_event_to_string(XEvent * event)
{
	g_return_val_if_fail(event, NULL);
	
	return g_strdup_printf("%s:\n\tserial=%lu\n\tsend_event=%s\n\tdisplay=%s\n\twindow=%d",
		xdk_util_event_get_name(event),
		event->xany.serial,
		event->xany.send_event ? "true" : "false",
		XDisplayString(event->xany.display),
		event->xany.window);
}

const char * xdk_util_event_get_name(XEvent * event)
{
	g_return_val_if_fail(event, event_names[0]);
	g_return_val_if_fail(event->type < 2 || event->type >= G_N_ELEMENTS(* event_names), event_names[0]);
	
	return event_names[event->type];
}
