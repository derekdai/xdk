#include "xdk-util.h"
#include "xdk-display.h"
#include "xdk-types.h"
#include <X11/keysym.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>

typedef void (* XQueryExtensionFunc)(Display * display, int * event_base, int error_base);

typedef void (* XEventToStringFunc)(XEvent * event, gchar * buf, gssize buf_size);

typedef struct _XEventInfo XEventInfo;

typedef struct _ExtensionInfo ExtensionInfo;

struct _XEventInfo
{
	const char * name;
	
	XEventToStringFunc to_string_func;
};

struct _ExtensionInfo
{
	const char * name;
	
	XQueryExtensionFunc query_extension_func;
	
	XEventToStringFunc event_to_string_func;

	int event_base;
	
	int error_base;
};

static XEventToStringFunc xdk_util_event_get_to_string_func(XEvent * event);
static void motion_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void crossing_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void focus_change_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void expose_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void graphics_expose_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void no_expose_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void visibility_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void create_window_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void destroy_window_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void unmap_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void map_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void map_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void reparent_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void configure_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void gravity_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void resize_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void configure_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void circulate_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void circulate_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void property_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void selection_clear_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void selection_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void selection_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void colormap_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void client_message_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void mapping_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void generic_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void error_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void keymap_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static gint window_to_string(Window window, gchar * buf, gssize buf_size);
static const char * boolean_to_string(gboolean v);
static const char * window_class_to_string(gint win_class);
static const char * backing_store_to_string(gint backing_store);
static const char * gravity_to_string(gint gravity);
static const char * map_state_to_string(gint map_state);
static void damage_event_to_string(XEvent * event, gchar * buf, gssize buf_size);
static void fixes_event_to_string(XEvent * event, gchar * buf, gssize buf_size);

static char xdk_util_buf[4096];

static const XEventInfo event_infos[] = {
	{ "Unknown", NULL },
	{ "Unknown", NULL },
	{ "KeyPress", motion_event_to_string },
	{ "KeyRelease", motion_event_to_string },
	{ "ButtonPress", motion_event_to_string },
	{ "ButtonRelease", motion_event_to_string },
	{ "MotionNotify", motion_event_to_string },
	{ "EnterNotify", crossing_event_to_string },
	{ "LeaveNotify", crossing_event_to_string },
	{ "FocusIn", focus_change_event_to_string },
	{ "FocusOut", focus_change_event_to_string },
	{ "KeymapNotify", keymap_event_to_string },
	{ "Expose", expose_event_to_string },
	{ "GraphicsExpose", graphics_expose_event_to_string },
	{ "NoExpose", no_expose_event_to_string },
	{ "VisibilityNotify", visibility_event_to_string },
	{ "CreateNotify", create_window_event_to_string },
	{ "DestroyNotify", destroy_window_event_to_string },
	{ "UnmapNotify", unmap_event_to_string },
	{ "MapNotify", map_event_to_string },
	{ "MapRequest", map_request_event_to_string },
	{ "ReparentNotify", reparent_event_to_string },
	{ "ConfigureNotify", configure_event_to_string },
	{ "ConfigureRequest", configure_request_event_to_string },
	{ "GravityNotify", gravity_event_to_string },
	{ "ResizeRequest", resize_request_event_to_string },
	{ "CirculateNotify", circulate_event_to_string },
	{ "CirculateRequest", circulate_request_event_to_string },
	{ "PropertyNotify", property_event_to_string },
	{ "SelectionClear", selection_clear_event_to_string },
	{ "SelectionRequest", selection_request_event_to_string },
	{ "SelectionNotify", selection_event_to_string },
	{ "ColormapNotify", colormap_event_to_string },
	{ "ClientMessage", client_message_event_to_string },
	{ "MappingNotify", mapping_event_to_string },
	{ "GenericEvent", generic_event_to_string },
};

static ExtensionInfo extension_infos[] = {
	{ "XDamage", XDamageQueryExtension, damage_event_to_string, 0, 0 },
	{ "XComposite", XCompositeQueryExtension, NULL, 0, 0 },
	{ "XFixes", XFixesQueryExtension, NULL, 0, 0 },
};

static void xdk_util_init()
{
	static volatile initialized = 0;
	
	if(g_once_init_enter(& initialized)) {
		Display * display = xdk_display_get_peer(xdk_display_get_default());
		int i = 0;
		for(; i < G_N_ELEMENTS(extension_infos); i ++) {
			extension_infos[i].query_extension_func(
				display,
				& extension_infos[i].event_base,
				& extension_infos[i].error_base);
			g_message("%s %d %d", extension_infos[i].name, extension_infos[i].event_base, extension_infos[i].error_base);
		}
		
		g_once_init_leave(& initialized, TRUE);
	}
}

void xdk_util_event_dump(XEvent * event)
{
	g_printerr("%s\n", xdk_util_event_to_string(event));
}

gchar * xdk_util_event_to_string(XEvent * event)
{
	g_return_val_if_fail(event, NULL);
	
	xdk_util_init();
	
	gint len = g_snprintf(
		xdk_util_buf, sizeof(xdk_util_buf),
		"%s %d:\n\tserial=%lu\n\tsend_event=%s\n\tdisplay=%s\n\tevent=",
		xdk_util_event_get_name(event),
		event->type,
		event->xany.serial,
		boolean_to_string(event->xany.send_event),
		XDisplayString(event->xany.display));
	len += window_to_string(event->xany.window, xdk_util_buf + len, sizeof(xdk_util_buf) - len);
		
	XEventToStringFunc to_string_func = xdk_util_event_get_to_string_func(event);
	if(to_string_func) {
		to_string_func(event, xdk_util_buf + len, sizeof(xdk_util_buf) - len);
	}
	
	return xdk_util_buf;
}

const char * xdk_util_event_get_name(XEvent * event)
{
	xdk_util_init();
	
	const char * name = "Unknown";
	if(! event) {
		name = "NULL event";
	}
	else if(event->type >= 2 && event->type < G_N_ELEMENTS(event_infos)) {
		name = event_infos[event->type].name;
	}
	else {
		int i = 0;
		for(; i < G_N_ELEMENTS(extension_infos); i ++) {
			if(event->type == extension_infos[i].event_base) {
				name = extension_infos[i].name;
				break;
			}
		}
	}
	
	return name;
}

static XEventToStringFunc xdk_util_event_get_to_string_func(XEvent * event)
{
	if(! event) {
		return NULL;
	}
	else if(event->type >= 2 && event->type < G_N_ELEMENTS(event_infos)) {
		return event_infos[event->type].to_string_func;
	}
	
	int i = 0;
	for(; i < G_N_ELEMENTS(extension_infos); i ++) {
		if(event->type == extension_infos[i].event_base) {
			return extension_infos[i].event_to_string_func;
		}
	}
	
	return NULL;
}

static gint state_to_string(guint state, gchar * buf, gssize buf_size)
{
	typedef struct _MaskNamePair
	{
		guint mask;
		const char * name;
	} MaskNamePair;
	static const MaskNamePair mask_name_pairs[] = {
		/*{ Button1Mask,	"Button1" },
		{ Button2Mask,	"Button2" },
		{ Button3Mask,	"Button3" },
		{ Button4Mask,	"Button4" },
		{ Button5Mask,	"Button5" },*/
		{ ShiftMask,	"Shift" },
		{ LockMask,		"Lock(CapsLock)" },
		{ ControlMask,	"Control" },
		{ Mod1Mask,		"Mod1(Alt)" },
		{ Mod2Mask,		"Mod2(CapsLock)" },
		{ Mod3Mask,		"Mod3" },
		{ Mod4Mask,		"Mod4" },
		{ Mod5Mask,		"Mod5" },
	};

	gint len = g_snprintf(buf, buf_size, "\n\tstate=");

	int i = 0;
	for(; i < G_N_ELEMENTS(mask_name_pairs); i ++) {
		if(state & mask_name_pairs[i].mask) {
			len += g_snprintf(buf + len, buf_size - len, "%s ", mask_name_pairs[i].name);
		}
	}
	
	return len;
}

static void motion_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XButtonEvent * e = (XButtonEvent *) event;
	gint len = g_snprintf(buf, buf_size,
		"\n\troot=%lu\n\tsubwindow=%lu\n\ttime=%lu\n\tx=%d, y=%d\n\tx_root=%d, y_root=%d\n\tsame_screen=%s",
		e->root,
		e->subwindow,
		e->time,
		e->x, e->y,
		e->x_root, e->y_root,
		boolean_to_string(e->same_screen));
	len += state_to_string(e->state, buf + len, buf_size - len);

	switch(event->type) {
	case ButtonPress:
	case ButtonRelease:
		g_snprintf(buf + len, buf_size - len,
			"\n\tbutton=Button%d",
			e->button);
		break;
	case KeyPress:
	case KeyRelease: {
		KeySym keysym = XLookupKeysym((XKeyEvent *) event, 0);
		g_snprintf(buf + len, buf_size - len,
			"\n\tkeycode=0x%x (%s)",
			((XKeyEvent *) event)->keycode,
			XKeysymToString(keysym));
		break;
		}
	case MotionNotify:
		g_snprintf(buf + len, buf_size - len,
			"\n\tis_hint=%s",
			(NotifyNormal == ((XMotionEvent *) e)->is_hint) ? "NotifyNormal" : "NotifyHint");
		break;
	}
}

static void crossing_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}


static void focus_change_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}


static void expose_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XExposeEvent * e = (XExposeEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\tx=%d y=%d\n\twidth=%d height=%d\n\tcount=%d",
		e->window,
		e->x, e->y,
		e->width, e->height,
		e->count);
}

static void graphics_expose_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XGraphicsExposeEvent * e = (XGraphicsExposeEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\tx=%d y=%d\n\twidth=%d height=%d\n\tcount=%d\n\tmajor_code=%d minor_code=%d",
		e->x, e->y,
		e->width, e->height,
		e->count,
		e->major_code, e->minor_code);
}


static void no_expose_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XGraphicsExposeEvent * e = (XGraphicsExposeEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\tmajor_code=%d minor_code=%d",
		e->major_code, e->minor_code);
}


static void visibility_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void create_window_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XCreateWindowEvent * e = (XCreateWindowEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\tx=%d y=%d\n\twidth=%d height=%d\n\tborder_width=%d\n\toverride_redirect=%s",
		e->window,
		e->x, e->y,
		e->width, e->height,
		e->border_width,
		boolean_to_string(e->override_redirect));
}

static void destroy_window_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XDestroyWindowEvent * e = (XDestroyWindowEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu", e->window);
}

static void unmap_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XUnmapEvent * e = (XUnmapEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\tfrom_configure=%s",
		e->window,
		boolean_to_string(e->from_configure));
}

static void map_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XMapEvent * e = (XMapEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\toverride_redirect=%s",
		e->window,
		boolean_to_string(e->override_redirect));
}

static void map_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XMapRequestEvent * e = (XMapRequestEvent *) event;
	g_snprintf(buf, buf_size, "\n\twindow=%lu", e->window);
}

static void reparent_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XReparentEvent * e = (XReparentEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\tparent=%lu\n\tx=%d, y=%d\n\toverride_redirect=%s",
		e->window,
		e->parent,
		e->x, e->y,
		boolean_to_string(e->override_redirect));
}

static void configure_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XConfigureEvent * e = (XConfigureEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\tx=%d, y=%d\n\twidth=%d, height=%d\n\tborder_width=%d\n\tabove(Window)=%lu\n\toverride_redirect=%s",
		e->window,
		e->x, e->y,
		e->width, e->height,
		e->border_width,
		e->above,
		boolean_to_string(e->override_redirect));
}

static void gravity_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XGravityEvent * e = (XGravityEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\tx=%d, y=%d", e->window, e->x, e->y);
}

static void resize_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XResizeRequestEvent * e = (XResizeRequestEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\twindow=%lu\n\twidth=%d, height=%d", e->window, e->width, e->height);
}

static void configure_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XConfigureRequestEvent * e = (XConfigureRequestEvent *) event;
	gint len = g_snprintf(buf, buf_size, "\n\tchild=%lu", e->window);
	if(e->value_mask & CWX) {
		len += g_snprintf(buf + len, buf_size - len, "\n\tx=%d", e->x);
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

static void circulate_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void circulate_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void property_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XPropertyEvent * e = (XPropertyEvent *) event;
	XdkDisplay * display = xdk_display_get_default();
	g_snprintf(buf, buf_size,
		"\n\tatom=%s\n\ttime=%lu\n\tstate=%s",
		xdk_display_atom_to_name(display, e->atom),
		e->time,
		(PropertyNewValue == e->state) ? "PropertyNewValue" : "PropertyDelete");
}

static void selection_clear_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void selection_request_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void selection_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void colormap_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void client_message_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XClientMessageEvent * e = (XClientMessageEvent *) event;
	XdkDisplay * display = xdk_display_get_default();
	const char * message_type = xdk_display_atom_to_name(display, e->message_type);
	gint len = g_snprintf(buf, buf_size,
		"\n\tmessage_type=%s\n\tformat=%d",
		message_type,
		e->format);
		
	if(! g_strcmp0("WM_PROTOCOLS", message_type)) {
		g_snprintf(buf + len, buf_size - len,
			"\n\tdata.l[0](protocols)=%s",
			xdk_display_atom_to_name(display, e->data.l[0]));
	}
	else if(! g_strcmp0("_NET_WM_STATE", message_type)) {
		g_snprintf(buf + len, buf_size - len,
			"\n\tdata.l[0](action)=%s\n\tdata.l[1](1st prop)=%s\n\tdata.l[2](2nd prop)=%s\n\tdata.l[3](source indication)=%s",
			e->data.l[0] == 0
				? "remove"
				: e->data.l[0] == 1
					? "add"
					: e->data.l[0] == 2
						? "toggle"
						: "unknown",
			xdk_display_atom_to_name(display, e->data.l[1]),
			e->data.l[2] == 0
				? ""
				: xdk_display_atom_to_name(display, e->data.l[2]),
			e->data.l[3] == 1
				? "normal application"
				: e->data.l[2] == 2
					? "pager"
					: "legacy");
	}
}

static void mapping_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void generic_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void error_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static void keymap_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	
}

static const char * boolean_to_string(gboolean v)
{
	return v ? "true" : "false";
}

static gint window_to_string(Window window, gchar * buf, gssize buf_size)
{
	XdkWindow * root = xdk_get_default_root_window();
	if(window == xdk_window_get_peer(root)) {
		return g_snprintf(buf, buf_size, "%lu (root)", window);
	}
	
	XdkDisplay * display = xdk_display_get_default();
	XWindowAttributes attributes;
	
	xdk_trap_error();
	XGetWindowAttributes(
		xdk_display_get_peer(display),
		window,
		& attributes);
	GError * error = NULL;
	if(xdk_untrap_error(& error)) {
		g_error_free(error);
		return g_snprintf(buf, buf_size, "%lu is not a window", window);
	}
	
	return g_snprintf(buf, buf_size,
		"[window=%lu, x=%d, y=%d, width=%d, height=%d, depth=%d, visual=%p, " \
		"root=%lu, class=%s, bit_gravity=%s, win_gravity=%s, " \
		"backing_store=%s, backing_planes=0x%08lx, backing_pixel=0x%08x, " \
		"save_under=%s, colormap=%lu, map_installed=%s, map_state=%s, " \
		"all_event_masks=%x, your_event_mask=%x, " \
		"do_not_propagate_mask=%x, override_redirect=%s, screen=%p]",
		window,
		attributes.x,
		attributes.y,
		attributes.width,
		attributes.height,
		attributes.depth,
		attributes.visual,
		attributes.root,
		window_class_to_string(attributes.class),
		gravity_to_string(attributes.bit_gravity),
		gravity_to_string(attributes.win_gravity),
		backing_store_to_string(attributes.backing_store),
		attributes.backing_planes,
		attributes.backing_pixel,
		boolean_to_string(attributes.save_under),
		attributes.colormap,
		boolean_to_string(attributes.map_installed),
		map_state_to_string(attributes.map_state),
		attributes.all_event_masks,
		attributes.your_event_mask,
		attributes.do_not_propagate_mask,
		boolean_to_string(attributes.override_redirect),
		attributes.screen);
}

static const char * window_class_to_string(gint win_class)
{
	return (InputOutput == win_class) ? "InputOutput" : "InputOnly";
}

static const char * backing_store_to_string(gint backing_store)
{
	switch(backing_store) {
	case NotUseful:
		return "NotUseful";
	case WhenMapped:
		return "WhenMapped";
	case Always:
		return "Always";
	}
	
	return "Unknown";
}

static const char * gravity_to_string(gint gravity)
{
	switch(gravity) {
	case UnmapGravity:
		return "UnmapGravity";
	case NorthWestGravity:
		return "NorthWestGravity";
	case NorthGravity:
		return "NorthGravity";
	case NorthEastGravity:
		return "NorthEastGravity";
	case WestGravity:
		return "WestGravity";
	case CenterGravity:
		return "CenterGravity";
	case EastGravity:
		return "EastGravity";
	case SouthWestGravity:
		return "SouthWestGravity";
	case SouthGravity:
		return "SouthGravity";
	case SouthEastGravity:
		return "SouthEastGravity";
//	case ForgetGravity:
//		return "ForgetGravity";
	}

	return "Unknown";
}

static const char * map_state_to_string(gint map_state)
{
	switch(map_state) {
	case IsUnmapped:
		return "IsUnmapped";
	case IsUnviewable:
		return "IsUnviewable";
	case IsViewable:
		return "IsViewable";
	}
	
	return "Unknown";
}

void xdk_util_window_dump(Window window)
{
	g_printerr("%s\n", xdk_util_window_to_string(window));
}

const char * xdk_util_window_to_string(Window window)
{
	window_to_string(window, xdk_util_buf, sizeof(xdk_util_buf));
	
	return xdk_util_buf;
}

static void damage_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
	XDamageNotifyEvent * e = (XDamageNotifyEvent *) event;
	g_snprintf(buf, buf_size,
		"\n\tdamage=%lu\n\tlevel=%d\n\tmore=%s\n\tarea=[%d,%d,%u,%u]\n\tgeometry=[%d,%d,%u,%u]",
		e->damage, e->level, boolean_to_string(e->more),
		e->area.x, e->area.y, e->area.width, e->area.height,
		e->geometry.x, e->geometry.y, e->geometry.width, e->geometry.height);
}

static void fixes_event_to_string(XEvent * event, gchar * buf, gssize buf_size)
{
}
