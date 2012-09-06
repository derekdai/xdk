#include "xdk-util.h"
#include "xdk-display.h"

typedef void (* XEventToStringFunc)(XEvent * event, gchar * buf, gsize buf_size);

typedef struct _XEventInfo XEventInfo;

struct _XEventInfo
{
	const char * name;
	
	XEventToStringFunc to_string_func;
};

static XEventToStringFunc xdk_util_event_get_to_string_func(XEvent * event);
static void key_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void button_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void motion_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void crossing_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void focus_change_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void expose_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void graphics_expose_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void no_expose_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void visibility_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void create_window_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void destroy_window_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void unmap_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void map_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void map_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void reparent_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void configure_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void gravity_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void resize_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void configure_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void circulate_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void circulate_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void property_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void selection_clear_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void selection_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void selection_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void colormap_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void client_message_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void mapping_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void generic_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void error_event_to_string(XEvent * event, gchar * buf, gsize buf_size);
static void keymap_event_to_string(XEvent * event, gchar * buf, gsize buf_size);

static char xdk_util_buf[1024];

static const XEventInfo event_infos[] = {
	{ "Unknown", NULL },
	{ "Unknown", NULL },
	{ "XDK_EVENT_KEY_PRESS (KeyPress)", key_event_to_string },
	{ "XDK_EVENT_KEY_RELEASE (KeyRelease)", key_event_to_string },
	{ "XDK_EVENT_BUTTON_PRESS (ButtonPress)", button_event_to_string },
	{ "XDK_EVENT_BUTTON_RELEASE (ButtonRelease)", button_event_to_string },
	{ "XDK_EVENT_MOTION (MotionNotify)", motion_event_to_string },
	{ "XDK_EVENT_ENTER (EnterNotify)", crossing_event_to_string },
	{ "XDK_EVENT_LEAVE (LeaveNotify)", crossing_event_to_string },
	{ "XDK_EVENT_FOCUS_IN (FocusIn)", focus_change_event_to_string },
	{ "XDK_EVENT_FOCUS_OUT (FocusOut)", focus_change_event_to_string },
	{ "XDK_EVENT_KEYMAP (KeymapNotify)", keymap_event_to_string },
	{ "XDK_EVENT_EXPOSE (Expose)", expose_event_to_string },
	{ "XDK_EVENT_GRAPHICS_EXPOSE (GraphicsExpose)", graphics_expose_event_to_string },
	{ "XDK_EVENT_NO_EXPOSE (NoExpose)", no_expose_event_to_string },
	{ "XDK_EVENT_VISIBILITY (VisibilityNotify)", visibility_event_to_string },
	{ "XDK_EVENT_CREATE (CreateNotify)", create_window_event_to_string },
	{ "XDK_EVENT_DESTROY (DestroyNotify)", destroy_window_event_to_string },
	{ "XDK_EVENT_UNMAP (UnmapNotify)", unmap_event_to_string },
	{ "XDK_EVENT_MAP (MapNotify)", map_event_to_string },
	{ "XDK_EVENT_MAP_REQUEST (MapRequest)", map_request_event_to_string },
	{ "XDK_EVENT_REPARENT (ReparentNotify)", reparent_event_to_string },
	{ "XDK_EVENT_CONFIGURE (ConfigureNotify)", configure_event_to_string },
	{ "XDK_EVENT_CONFIGURE_REQUEST (ConfigureRequest)", configure_request_event_to_string },
	{ "XDK_EVENT_GRAVITY (GravityNotify)", gravity_event_to_string },
	{ "XDK_EVENT_RESIZE_REQUEST (ResizeRequest)", resize_request_event_to_string },
	{ "XDK_EVENT_CIRCULATE (CirculateNotify)", circulate_event_to_string },
	{ "XDK_EVENT_CIRCULATE_REQUEST = CirculateRequest)", circulate_request_event_to_string },
	{ "XDK_EVENT_PROPERTY (PropertyNotify)", property_event_to_string },
	{ "XDK_EVENT_SELECTION_CLEAR (SelectionClear)", selection_clear_event_to_string },
	{ "XDK_EVENT_SELECTION_REQUEST (SelectionRequest)", selection_request_event_to_string },
	{ "XDK_EVENT_SELECTION (SelectionNotify)", selection_event_to_string },
	{ "XDK_EVENT_COLORMAP (ColormapNotify)", colormap_event_to_string },
	{ "XDK_EVENT_CLIENT_MESSAGE (ClientMessage)", client_message_event_to_string },
	{ "XDK_EVENT_MAPPING (MappingNotify)", mapping_event_to_string },
	{ "XDK_EVENT_GENERIC (GenericEvent)", generic_event_to_string },
};

void xdk_util_event_dump(XEvent * event)
{
	g_printerr("%s\n", xdk_util_event_to_string(event));
}

gchar * xdk_util_event_to_string(XEvent * event)
{
	g_return_val_if_fail(event, NULL);
	
	gint len = g_snprintf(
		xdk_util_buf, sizeof(xdk_util_buf),
		"%s:\n\tserial=%lu\n\tsend_event=%s\n\tdisplay=%s\n\tevent=%lu",
		xdk_util_event_get_name(event),
		event->xany.serial,
		event->xany.send_event ? "true" : "false",
		XDisplayString(event->xany.display),
		event->xany.window);
		
	XEventToStringFunc to_string_func = xdk_util_event_get_to_string_func(event);
	if(to_string_func) {
		to_string_func(event, xdk_util_buf + len, sizeof(xdk_util_buf) - len);
	}
	
	return xdk_util_buf;
}

const char * xdk_util_event_get_name(XEvent * event)
{
	g_return_val_if_fail(event, "Unknown");
	g_return_val_if_fail(event->type >= 2 && event->type < G_N_ELEMENTS(event_infos), "Unknown");
	
	return event_infos[event->type].name;
}

static XEventToStringFunc xdk_util_event_get_to_string_func(XEvent * event)
{
	g_return_val_if_fail(event, NULL);
	g_return_val_if_fail(event->type >= 2 && event->type < G_N_ELEMENTS(event_infos), NULL);
	
	return event_infos[event->type].to_string_func;
}

static void key_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void button_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void motion_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void crossing_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}


static void focus_change_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}


static void expose_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}


static void graphics_expose_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}


static void no_expose_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}


static void visibility_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}


static void create_window_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}


static void destroy_window_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}


static void unmap_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void map_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	XMapEvent * e = (XMapEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\toverride_redirect=%s",
		e->window,
		e->override_redirect ? "true" : "false");
}

static void map_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void reparent_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	XReparentEvent * e = (XReparentEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\tparent=%lu\n\tx=%d, y=%d\n\toverride_redirect=%s",
		e->window,
		e->parent,
		e->x, e->y,
		e->override_redirect ? "true" : "false");
}

static void configure_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	XConfigureEvent * e = (XConfigureEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\tx=%d, y=%d\n\twidth=%d, height=%d\n\tborder_width=%d\n\tabove(Window)=%lu\n\toverride_redirect=%s",
		e->window,
		e->x, e->y,
		e->width, e->height,
		e->border_width,
		e->above,
		e->override_redirect ? "true" : "false");
}

static void gravity_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void resize_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void configure_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	XConfigureRequestEvent * e = (XConfigureRequestEvent *) event;
	gint len = g_snprintf(buf, buf_size, "\n\tchild=%lu", e->window);
	if(e->value_mask & CWX) {
		gint len = g_snprintf(buf + len, buf_size - len, "\n\tx=%d", e->x);
	}
	if(e->value_mask & CWY) {
		len += g_snprintf(buf + len, buf_size - len, "\n\ty=%d", e->y);
	}
	if(e->value_mask & CWWidth) {
		len += g_snprintf(buf + len, buf_size - len, "\n\twidth=%d", e->width);
	}
	if(e->value_mask & CWHeight) {
		len += g_snprintf(buf + len, buf_size - len, "\n\theight=%d", e->height);
	}
	if(e->value_mask & CWBorderWidth) {
		len += g_snprintf(buf + len, buf_size - len, "\n\tborder_width=%d", e->border_width);
	}
	if(e->value_mask & CWSibling) {
		len += g_snprintf(buf + len, buf_size - len, "\n\tabove=%lu", e->above);
	}
	if(e->value_mask & CWStackMode) {
		len += g_snprintf(buf + len, buf_size - len, "\n\tdetail=0x%x", e->detail);
	}
}

static void circulate_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void circulate_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void property_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void selection_clear_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void selection_request_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void selection_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void colormap_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void client_message_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	XClientMessageEvent * e = (XClientMessageEvent *) event;
	XdkDisplay * display = xdk_display_get_default();
	g_snprintf(buf, buf_size,
		"\n\tmessage_type=%s\n\tformat=%d",
		xdk_display_atom_to_name(display, e->message_type),
		e->format);
}

static void mapping_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void generic_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void error_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}

static void keymap_event_to_string(XEvent * event, gchar * buf, gsize buf_size)
{
	
}
