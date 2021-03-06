#include "xdk-window.h"
#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-gc.h"
#include "xdk-visual.h"

#define XDK_WINDOW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE(o, XDK_TYPE_WINDOW, XdkWindowPrivate))

struct _XdkWindowPrivate
{
	Window peer;
	
	XdkDisplay * display;
	
	XdkScreen * screen;
	
	XdkWindow * parent;
	
	gint x;
	
	gint y;
	
	gint width;
	
	gint height;
	
	guint border_width;
	
	XdkVisual * visual;
	
	gulong background_color;
	
	gulong event_mask;
	
	XdkGravity gravity;
	
	GList * children;
	
	gboolean mapped : 1;
	
	gboolean visible : 1;
	
	gboolean own_peer : 1;
	
	gboolean destroyed : 1;
	
	gboolean override_redirect : 1;
	
	gboolean maximized : 1;
	
	gboolean keep_above : 1;
	
	gboolean keep_below : 1;
	
	gboolean fullscreen : 1;
};

enum {
	XDK_WINDOW_DELETE_EVENT = XDK_EVENT_LAST,
	XDK_WINDOW_DESTROY,
	SIGNAL_MAX,
};

enum {
	PROP_SCREEN = 1,
};

static void xdk_window_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec);

static void xdk_window_constructed(GObject * object);

static void xdk_window_dispose(GObject * object);

static void xdk_window_finalize(GObject * object);

static void xdk_window_default_realize(XdkWindow * self);

static void xdk_window_default_unrealize(XdkWindow * self);

static void xdk_window_handle_delete_event(XdkWindow * self, XEvent * event);

static void xdk_window_handle_configure_notify(XdkWindow * self, XEvent * event);

static void xdk_window_handle_map_notify(XdkWindow * self, XEvent * event);

static void xdk_window_handle_unmap_notify(XdkWindow * self, XEvent * event);

static void xdk_window_handle_gravity_notify(XdkWindow * self, XEvent * event);

static void xdk_window_destroy_internal(XdkWindow * self);

static void xdk_window_sync_attributes(XdkWindow * self);

static void xdk_window_sync_net_wm_state(XdkWindow * self);

static gboolean contains_atom(
	XdkWindow * self,
	Atom * values, gint n_values,
	GQuark value_key);
	
static void xdk_window_normalize_size(
	XdkWindow * self,
	gint * width,
	gint * height);

G_DEFINE_TYPE(XdkWindow, xdk_window, G_TYPE_OBJECT);

static guint signals[SIGNAL_MAX] = { 0, };

static void xdk_window_class_init(XdkWindowClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->set_property = xdk_window_set_property;
	gobject_class->constructed = xdk_window_constructed;
	gobject_class->dispose = xdk_window_dispose;
	gobject_class->finalize = xdk_window_finalize;
	
	clazz->realize = xdk_window_default_realize;
	clazz->unrealize = xdk_window_default_unrealize;
	clazz->delete_event = xdk_window_handle_delete_event;
	clazz->map_notify = xdk_window_handle_map_notify;
	clazz->unmap_notify = xdk_window_handle_unmap_notify;
	
	g_object_class_install_property(
		gobject_class,
		PROP_SCREEN,
		g_param_spec_object(
			"screen", "", "",
			XDK_TYPE_SCREEN,
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
	
	GType type = G_TYPE_FROM_CLASS(clazz);
	signals[XDK_WINDOW_DELETE_EVENT] = g_signal_new(
		"delete-event",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, delete_event),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_WINDOW_DESTROY] = g_signal_new(
		"destroy",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, destroy),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 0);

	signals[XDK_EVENT_DESTROY] = g_signal_new(
		"destroy-notify",
		type,
		G_SIGNAL_RUN_LAST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);

	signals[XDK_EVENT_CREATE] = g_signal_new(
		"create-notify",
		type,
		G_SIGNAL_RUN_LAST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);

	signals[XDK_EVENT_REPARENT] = g_signal_new(
		"reparent-notify",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_CONFIGURE] = g_signal_new(
		"configure-notify",
		type,
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkWindowClass, configure_notify),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_MAP] = g_signal_new(
		"map-notify",
		type,
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkWindowClass, map_notify),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_UNMAP] = g_signal_new(
		"unmap-notify",
		type,
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkWindowClass, map_notify),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_GRAVITY] = g_signal_new(
		"gravity-notify",
		type,
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkWindowClass, gravity_notify),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_KEY_PRESS] = g_signal_new(
		"key-press",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, key_press),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_KEY_RELEASE] = g_signal_new(
		"key-release",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, key_release),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_BUTTON_PRESS] = g_signal_new(
		"button-press",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, button_press),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_BUTTON_RELEASE] = g_signal_new(
		"button-release",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, button_release),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_MOTION] = g_signal_new(
		"motion",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, motion),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_ENTER] = g_signal_new(
		"enter",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, enter),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_LEAVE] = g_signal_new(
		"leave",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, leave),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_FOCUS_IN] = g_signal_new(
		"focus-in",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_FOCUS_OUT] = g_signal_new(
		"focus-out",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_EXPOSE] = g_signal_new(
		"expose",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_CONFIGURE_REQUEST] = g_signal_new(
		"configure-request",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_MAP_REQUEST] = g_signal_new(
		"map-request",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_RESIZE_REQUEST] = g_signal_new(
		"resize-request",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_CIRCULATE_REQUEST] = g_signal_new(
		"circulate-request",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_PROPERTY] = g_signal_new(
		"property",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	/*
	case XDK_EVENT_KEYMAP:
	case XDK_EVENT_GRAPHICS_EXPOSE:
	case XDK_EVENT_NO_EXPOSE:
	case XDK_EVENT_VISIBILITY:
	case XDK_EVENT_CIRCULATE:
	case XDK_EVENT_SELECTION_CLEAR:
	case XDK_EVENT_SELECTION_REQUEST:
	case XDK_EVENT_SELECTION:
	case XDK_EVENT_COLORMAP:
	case XDK_EVENT_MAPPING:
	case XDK_EVENT_GENERIC:
	*/
		
	g_type_class_add_private(clazz, sizeof(XdkWindowPrivate));
}

static void xdk_window_init(XdkWindow * self)
{
	XdkWindowPrivate * priv = XDK_WINDOW_GET_PRIVATE(self);
	self->priv = priv;
	
	priv->peer = None;
	priv->width = 1;
	priv->height = 1;
	priv->gravity = XDK_GRAVITY_CENTER;
	priv->event_mask = XDK_EVENT_MASK_STRUCTURE_NOTIFY;
}

static void xdk_window_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec)
{
	XdkWindow * self = XDK_WINDOW(object);
	
	switch(property_id) {
	case PROP_SCREEN: {
		XdkScreen * screen = XDK_SCREEN(g_value_get_object(value));
		if(NULL == screen) {
			screen = xdk_get_default_screen();
		}
		xdk_window_set_screen(self, screen);
		break;
	}
	default:
		g_return_if_reached();
	}
}

static void xdk_window_constructed(GObject * object)
{
	G_OBJECT_CLASS(xdk_window_parent_class)->constructed(object);
	
	XdkWindow * self = XDK_WINDOW(object);
	XdkWindowPrivate * priv = self->priv;
	if(! priv->screen) {
		priv->screen = xdk_get_default_screen();
	}

	priv->background_color = xdk_screen_get_white(priv->screen);
	xdk_window_set_visual(self, xdk_screen_get_default_visual(priv->screen));
}

static void xdk_window_dispose(GObject * object)
{
	XdkWindow * self = XDK_WINDOW(object);
	XdkWindowPrivate * priv = self->priv;
	
	xdk_window_destroy_internal(self);
	
	g_object_unref(priv->visual);
	priv->visual = NULL;
}

static void xdk_window_finalize(GObject * object)
{
	G_OBJECT_CLASS(xdk_window_parent_class)->finalize(object);
}

XdkWindow * xdk_window_new()
{
	return g_object_new(
		XDK_TYPE_WINDOW,
		"screen", xdk_get_default_screen(),
		NULL);
}

static void _xdk_window_set_peer(XdkWindow * self, Window peer, gboolean own_peer)
{
	g_return_if_fail(self);

	XdkWindowPrivate * priv = self->priv;
	if(peer == priv->peer) {
		return;
	}
	
	if(None != priv->peer) {
		xdk_display_remove_window(priv->display, self);
	}
	
	priv->peer = peer;
	priv->own_peer = own_peer;
	
	if(None != priv->peer) {
		xdk_display_add_window(priv->display, self);
	}
}

static void xdk_window_sync_attributes(XdkWindow * self)
{
	XWindowAttributes attributes;
	xdk_window_get_attributes(self, & attributes);
	
	XdkWindowPrivate * priv = self->priv;
	priv->x = attributes.x;
	priv->y = attributes.y;
	priv->width = attributes.width;
	priv->height = attributes.height;
	priv->gravity = attributes.win_gravity;
	priv->mapped = (IsUnmapped != attributes.map_state);
	priv->event_mask = attributes.your_event_mask;
	priv->override_redirect = !! attributes.override_redirect;
}

void xdk_window_set_foreign_peer(XdkWindow * self, Window peer)
{
	_xdk_window_set_peer(self, peer, FALSE);
	
	xdk_window_sync_attributes(self);
	xdk_window_sync_net_wm_state(self);
}

void xdk_window_take_peer(XdkWindow * self, Window peer)
{
	_xdk_window_set_peer(self, peer, TRUE);
	
	xdk_window_sync_attributes(self);
	xdk_window_sync_net_wm_state(self);
}

Window xdk_window_get_peer(XdkWindow * self)
{
	g_return_val_if_fail(self, None);
	
	return self->priv->peer;
}

gboolean xdk_window_is_realized(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return ! self->priv->destroyed && None != self->priv->peer;
}

/**
 * http://tronche.com/gui/x/xlib/window/XCreateWindow.html
 */
Window xdk_window_create_window(
	XdkWindow * self,
	XdkWindow * parent,
	XdkWindowClasses window_type,
	XdkWindowAttributeMask attribute_mask,
	XSetWindowAttributes * attributes)
{
	g_return_if_fail(self);
	g_return_if_fail(parent);
	g_return_if_fail(attributes);
	
	XdkWindowPrivate * priv = self->priv;
	XSetWindowAttributes tmp = * attributes;
	if(! (CWColormap & attribute_mask)) {
		attribute_mask |= CWColormap;
		attributes = & tmp;
		attributes->colormap = xdk_visual_to_colormap(priv->visual);
	}
	
	xdk_trap_error();
	Window window = XCreateWindow(
		xdk_display_get_peer(priv->display),
		xdk_window_get_peer(parent),
		priv->x, priv->y,
		priv->width, priv->height,
		priv->border_width,
		xdk_visual_get_depth(priv->visual),
		window_type,
		xdk_visual_get_peer(priv->visual),
		attribute_mask,
		attributes);
	xdk_flush();
		
	GError * error = NULL;
	if(xdk_untrap_error(& error)) {
		if(error) {
			g_warning("Failed to create window: %s", error->message);
			g_error_free(error);
		}
	}
	
	return window;
}

void xdk_window_realize(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(xdk_window_is_realized(self)) {
		return;
	}
	
	XDK_WINDOW_GET_CLASS(self)->realize(self);
}

static void xdk_window_default_realize(XdkWindow * self)
{
	XdkWindowPrivate * priv = self->priv;
	XSetWindowAttributes attributes = {
		.border_pixel = 0,
		.background_pixel = priv->background_color,
		.win_gravity = priv->gravity,
		.backing_store = WhenMapped,
		.event_mask = priv->event_mask,
	};
	Window peer = xdk_window_create_window(
		self,
		priv->parent ? priv->parent : xdk_screen_get_root_window(priv->screen),
		XDK_WINDOW_CLASSES_INPUT_OUTPUT,
		CWBorderPixel | CWBackPixel | CWWinGravity | CWBackingStore |
			CWEventMask,
		& attributes);
	if(None == peer) {
		return;
	}
	xdk_window_take_peer(self, peer);

	// involve WM_DELETE_WINDOW client event
	Atom atom = xdk_display_atom_get(
		priv->display,
		XDK_ATOM_WM_DELETE_WINDOW);
	xdk_window_set_wm_protocols(self, & atom, 1);
}

static void xdk_window_default_unrealize(XdkWindow * self)
{
	XdkWindowPrivate * priv = self->priv;
	XDestroyWindow(
		xdk_display_get_peer(priv->display),
		priv->peer);
	xdk_display_flush(priv->display);
}

void xdk_window_unrealize(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(! xdk_window_is_realized(self)) {
		return;
	}
	
	xdk_window_unmap(self);
	XDK_WINDOW_GET_CLASS(self)->unrealize(self);

	g_signal_emit(self, signals[XDK_WINDOW_DESTROY], 0);
	
	_xdk_window_set_peer(self, None, priv->own_peer);
}

void xdk_window_get_position(XdkWindow * self, int * x, int * y)
{
	g_return_if_fail(self);
	
	if(x) {
		* x = self->priv->x;
	}
	
	if(y) {
		* y = self->priv->y;
	}
}

void xdk_window_set_position(XdkWindow * self, int x, int y)
{
	g_return_if_fail(self);

	XdkWindowPrivate * priv = self->priv;
	priv->x = x;
	priv->y = y;
	
	if(! xdk_window_is_realized(self)) {
		return;
	}
	
	XMoveWindow(
		xdk_display_get_peer(priv->display), priv->peer,
		x, y);
}

int xdk_window_get_x(XdkWindow * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->x;
}

void xdk_window_set_x(XdkWindow * self, int x)
{
	g_return_if_fail(self);
	
	xdk_window_set_position(self, x, self->priv->y);
}

int xdk_window_get_y(XdkWindow * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->y;
}

void xdk_window_set_y(XdkWindow * self, int y)
{
	g_return_if_fail(self);
	
	xdk_window_set_position(self, self->priv->x, y);
}

void xdk_window_get_size(XdkWindow * self, gint * width, gint * height)
{
	g_return_if_fail(self);

	if(width) {
		* width = self->priv->width;
	}
	
	if(height) {
		* height = self->priv->height;
	}
}

void xdk_window_set_size(XdkWindow * self, gint width, gint height)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	xdk_window_set_geometry(self, priv->x, priv->y, width, height);
}

gint xdk_window_get_width(XdkWindow * self)
{
	g_return_val_if_fail(self, 1);
	
	return self->priv->width;
}

void xdk_window_set_width(XdkWindow * self, gint width)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	xdk_window_set_geometry(self, priv->x, priv->y, width, priv->height);
}

gint xdk_window_get_height(XdkWindow * self)
{
	g_return_val_if_fail(self, 1);
	
	return self->priv->height;
}

void xdk_window_set_height(XdkWindow * self, gint height)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	xdk_window_set_geometry(self, priv->x, priv->y, priv->width, height);
}

void xdk_window_get_geometry(
	XdkWindow * self,
	gint * x, gint * y,
	gint * width, gint * height)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(x) {
		* x = priv->x;
	}
	
	if(y) {
		* y = priv->y;
	}
	
	if(width) {
		* width = priv->width;
	}
	
	if(height) {
		* height = priv->height;
	}
}

static void xdk_window_normalize_size(
	XdkWindow * self,
	gint * width,
	gint * height)
{
	XdkWindowPrivate * priv = self->priv;
	if(width && -1 == * width) {
		* width = priv->parent
			? xdk_window_get_width(priv->parent)
			: xdk_screen_get_width(priv->screen);
	}
	
	if(height && -1 == * height) {
		* height = priv->parent
			? xdk_window_get_height(priv->parent)
			: xdk_screen_get_height(priv->screen);
	}
}

void xdk_window_set_geometry(
	XdkWindow * self,
	gint x, gint y,
	gint width, gint height)
{
	g_return_if_fail(self);
	g_return_if_fail(width >= -1 && width != 0);
	g_return_if_fail(height >= -1 && height != 0);
	
	XdkWindowPrivate * priv = self->priv;
	xdk_window_normalize_size(self, & width, & height);
	
	if(xdk_window_is_realized(self)) {
		XMoveResizeWindow(
			xdk_display_get_peer(priv->display),
			priv->peer,
			x, y,
			width, height);
	}
	
	priv->x = x;
	priv->y = y;
	priv->width = width;
	priv->height = height;
}

void xdk_window_map(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(xdk_window_is_mapped(self)) {
		return;
	}
	
	xdk_window_realize(self);
	XMapWindow(xdk_display_get_peer(priv->display), priv->peer);
	priv->mapped = TRUE;
}

gboolean xdk_window_is_mapped(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	XdkWindowPrivate * priv = self->priv;
	return ! priv->destroyed && priv->mapped;
}

void xdk_window_unmap(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! xdk_window_is_mapped(self)) {
		return;
	}
	
	XdkWindowPrivate * priv = self->priv;
	XUnmapWindow(xdk_display_get_peer(priv->display), priv->peer);
	priv->mapped = FALSE;
}

void xdk_window_show(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(priv->visible) {
		return;
	}
	
	xdk_window_realize(self);
	xdk_window_map(self);
	
	priv->visible = TRUE;
}

void xdk_window_show_all(XdkWindow * self)
{
	g_return_if_fail(self);
	
	xdk_window_show(self);
	
	g_list_foreach(self->priv->children, (GFunc) xdk_window_show, NULL);
}

void xdk_window_hide(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(! priv->visible) {
		return;
	}
	
	//xdk_window_unmap(self);
	
	priv->visible = FALSE;
}

void xdk_window_set_visible(XdkWindow * self, gboolean visible)
{
	if(visible) {
		xdk_window_show(self);
	}
	else {
		xdk_window_hide(self);
	}
}

gboolean xdk_window_get_visible(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return self->priv->visible && xdk_window_is_mapped(self);
}

gboolean xdk_window_is_destroyed(XdkWindow * self)
{
	g_return_val_if_fail(self, TRUE);
	
	return self->priv->destroyed;
}

static void xdk_window_destroy_internal(XdkWindow * self)
{
	XdkWindowPrivate * priv = self->priv;

	g_list_foreach(priv->children, (GFunc) xdk_window_destroy, NULL);
	if(priv->parent) {
		xdk_window_remove_child(priv->parent, self);
	}

	if(priv->own_peer) {
		xdk_window_unmap(self);
		xdk_window_unrealize(self);
	}
	else {
		_xdk_window_set_peer(self, None, FALSE);
	}
	
	self->priv->destroyed = TRUE;
}

void xdk_window_destroy(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(xdk_window_is_destroyed(self)) {
		return;
	}
	
	xdk_window_destroy_internal(self);

	g_object_unref(self);
}

Atom * xdk_window_list_properties(XdkWindow * self, gint * n_props)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(n_props, NULL);
	
	xdk_window_realize(self);
	
	Atom * orig = XListProperties(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer,
		n_props);
	Atom * retval = g_memdup(orig, sizeof(Atom) * (* n_props));
	XFree(orig);
	
	return retval;
}

static void xdk_window_dispatch_client_message(XdkWindow * self, XEvent * event)
{
	if(event->xclient.data.l[0] ==
			xdk_display_atom_get(self->priv->display, XDK_ATOM_WM_DELETE_WINDOW)) {
		g_signal_emit(self, signals[XDK_WINDOW_DELETE_EVENT], 0, event);
	}
}

void xdk_window_dispatch_event(XdkWindow * self, XEvent * event)
{
	gboolean retval = FALSE;
	
	switch(event->type) {
	case XDK_EVENT_KEY_PRESS:
	case XDK_EVENT_KEY_RELEASE:
	case XDK_EVENT_BUTTON_PRESS:
	case XDK_EVENT_BUTTON_RELEASE:
	case XDK_EVENT_MOTION:
	case XDK_EVENT_ENTER:
	case XDK_EVENT_LEAVE:
	case XDK_EVENT_FOCUS_IN:
	case XDK_EVENT_FOCUS_OUT:
	case XDK_EVENT_KEYMAP:
	case XDK_EVENT_EXPOSE:
	case XDK_EVENT_GRAPHICS_EXPOSE:
	case XDK_EVENT_NO_EXPOSE:
	case XDK_EVENT_VISIBILITY:
	case XDK_EVENT_CREATE:
	case XDK_EVENT_UNMAP:
	case XDK_EVENT_MAP:
	case XDK_EVENT_MAP_REQUEST:
	case XDK_EVENT_CONFIGURE:
	case XDK_EVENT_CONFIGURE_REQUEST:
	case XDK_EVENT_GRAVITY:
	case XDK_EVENT_REPARENT:
	case XDK_EVENT_RESIZE_REQUEST:
	case XDK_EVENT_CIRCULATE:
	case XDK_EVENT_CIRCULATE_REQUEST:
	case XDK_EVENT_PROPERTY:
	case XDK_EVENT_SELECTION_CLEAR:
	case XDK_EVENT_SELECTION_REQUEST:
	case XDK_EVENT_SELECTION:
	case XDK_EVENT_COLORMAP:
	case XDK_EVENT_MAPPING:
	case XDK_EVENT_GENERIC:
	case XDK_EVENT_DESTROY:
		g_signal_emit(self, signals[event->type], 0, event);
		break;
	case XDK_EVENT_CLIENT_MESSAGE:
		xdk_window_dispatch_client_message(self, event);
		break;
	default:
		g_return_if_reached();
	}
}

void xdk_window_set_gravity(XdkWindow * self, XdkGravity gravity)
{
}

XdkGravity xdk_window_get_gravity(XdkWindow * self)
{
}

void xdk_window_set_attributes(
	XdkWindow * self,
	XdkWindowAttributeMask mask,
	XSetWindowAttributes * attributes)
{
	g_return_if_fail(self);
	g_return_if_fail(attributes);
	g_return_if_fail(xdk_window_is_realized(self));
	
	XChangeWindowAttributes(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer,
		mask,
		attributes);
}

void xdk_window_get_attributes(
	XdkWindow * self,
	XWindowAttributes * attributes)
{
	g_return_if_fail(self);
	g_return_if_fail(attributes);
	
	XGetWindowAttributes(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer,
		attributes);
}

void xdk_window_set_parent(XdkWindow * self, XdkWindow * parent)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(priv->parent == parent) {
		return;
	}
	
	if(priv->parent) {
		xdk_window_remove_child(priv->parent, self);
	}
	
	priv->parent = parent;
	if(! priv->parent) {
		xdk_window_unmap(self);
		return;
	}
	
	xdk_window_realize(priv->parent);
	/*XReparentWindow(
		xdk_display_get_peer(priv->display),
		priv->peer,
		xdk_window_get_peer(priv->parent),
		priv->x, priv->y);*/
}

XdkWindow * xdk_window_get_parent(XdkWindow * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->parent;
}

void xdk_window_set_background_color(XdkWindow * self, gulong background_color)
{
	if(background_color == self->priv->background_color) {
		return;
	}
	
	XdkWindowPrivate * priv = self->priv;
	priv->background_color = background_color;
	if(! xdk_window_is_realized(self)) {
		return;
	}
	
	XSetWindowBackground(
		xdk_display_get_peer(priv->display),
		priv->peer,
		background_color);
	XClearWindow(xdk_display_get_peer(priv->display), priv->peer);
}

gulong xdk_window_get_background_color(XdkWindow * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->background_color;
}

static void xdk_window_event_mask_update(XdkWindow * self, XdkEventMask event_mask)
{
	XSetWindowAttributes attributes = {
		.event_mask = event_mask
	};
	xdk_window_set_attributes(self, XDK_ATTR_MASK_EVENT_MASK, & attributes);
}

void xdk_window_event_mask_set(XdkWindow * self, XdkEventMask event_mask)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	priv->event_mask = event_mask;
	
	if(xdk_window_is_realized(self)) {
		xdk_window_event_mask_update(self, priv->event_mask);
	}
}

void xdk_window_event_mask_add(XdkWindow * self, XdkEventMask event_mask)
{
	g_return_if_fail(self);
	
	self->priv->event_mask |= event_mask;
	xdk_window_event_mask_update(self, self->priv->event_mask);
}

void xdk_window_event_mask_remove(XdkWindow * self, XdkEventMask event_mask)
{
	g_return_if_fail(self);
	
	self->priv->event_mask &= ~event_mask;
	xdk_window_event_mask_update(self, self->priv->event_mask);
}

gulong xdk_window_event_mask_get(XdkWindow * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->event_mask;
}

XdkDisplay * xdk_window_get_display(XdkWindow * self)
{
	return self->priv->display;
}

XdkScreen * xdk_window_get_screen(XdkWindow * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->screen;
}

void xdk_window_set_screen(XdkWindow * self, XdkScreen * screen)
{
	g_return_if_fail(self);
	g_return_if_fail(screen);
	
	XdkWindowPrivate * priv = self->priv;
	if(screen == priv->screen) {
		return;
	}

	priv->screen = screen;
	priv->display = xdk_screen_get_display(screen);
	
	// reparent to new root window
}

void xdk_window_set_wm_protocols(XdkWindow * self, Atom * protocols, gint n_protocols)
{
	g_return_if_fail(self);
	
	XSetWMProtocols(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer,
		protocols, n_protocols);
}

Atom * xdk_window_get_wm_protocols(XdkWindow * self, gint * n_protocols)
{
	g_return_val_if_fail(self, NULL);
	
	gint tmp;
	if(! n_protocols) {
		n_protocols = & tmp;
	}
	
	Atom * protocols = NULL;
	XGetWMProtocols(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer,
		& protocols, n_protocols);
	
	return protocols;
}

static void xdk_window_handle_delete_event(XdkWindow * self, XEvent * event)
{
	xdk_window_destroy(self);
}

static void xdk_window_handle_configure_notify(XdkWindow * self, XEvent * event)
{
	XdkWindowPrivate * priv = self->priv;
	priv->x = event->xconfigure.x;
	priv->y = event->xconfigure.y;
	priv->width = event->xconfigure.height;
	priv->height = event->xconfigure.height;
}

static void xdk_window_handle_map_notify(XdkWindow * self, XEvent * event)
{
	self->priv->mapped = TRUE;
}

static void xdk_window_handle_unmap_notify(XdkWindow * self, XEvent * event)
{
	g_message("%lu unmapped", self->priv->peer);
	
	self->priv->mapped = FALSE;
}

void xdk_window_raise(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! xdk_window_is_realized(self)) {
		return;
	}
	
	XRaiseWindow(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer);
}

void xdk_window_lower(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! xdk_window_is_realized(self)) {
		return;
	}
	
	XLowerWindow(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer);
}

void xdk_window_set_borser_width(XdkWindow * self, guint width)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	priv->border_width = width;
	
	if(! xdk_window_is_realized(self)) {
		return;
	}
	
	XSetWindowBorderWidth(
		xdk_display_get_peer(priv->display),
		priv->peer,
		width);
}

guint xdk_window_get_borser_width(XdkWindow * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->border_width;
}

void xdk_window_add_child(XdkWindow * self, XdkWindow * child)
{
	g_return_if_fail(self);
	g_return_if_fail(child);
	
	if(xdk_window_contains_child(self, child)) {
		return;
	}
	
	XdkWindowPrivate * priv = self->priv;
	priv->children = g_list_prepend(priv->children, g_object_ref(child));
	xdk_window_set_parent(child, self);
}

void xdk_window_remove_child(XdkWindow * self, XdkWindow * child)
{
	g_return_if_fail(self);
	g_return_if_fail(child);
	
	XdkWindowPrivate * priv = self->priv;
	GList * node = g_list_find(priv->children, child);
	if(NULL == node) {
		return;
	}
	
	priv->children = g_list_delete_link(priv->children, node);
	g_object_unref(child);
	xdk_window_set_parent(self, NULL);
}

gboolean xdk_window_contains_child(XdkWindow * self, XdkWindow * child)
{
	g_return_val_if_fail(self, FALSE);
	g_return_val_if_fail(child, FALSE);

	return NULL != g_list_find(self->priv->children, child);
}

GList * xdk_window_list_children(XdkWindow * self)
{
	g_return_val_if_fail(self, NULL);

	return g_list_copy(self->priv->children);
}

void xdk_window_select_input(XdkWindow * self, XdkEventMask event_mask)
{
	g_return_if_fail(self);
	
	xdk_window_realize(self);
	
	XSelectInput(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer,
		event_mask);
}

GList * xdk_window_query_tree(XdkWindow * self)
{
	g_return_val_if_fail(self, NULL);
	XdkWindowPrivate * priv = self->priv;
	g_return_val_if_fail(priv->peer, NULL);
	
	GList * windows = NULL;
	Window xroot, xparent, * xwindows;
	guint n_xwindows;
	if(! XQueryTree(
			xdk_display_get_peer(priv->display),
			priv->peer,
			& xroot,
			& xparent,
			& xwindows, & n_xwindows)) {
		goto end;
	}
	
	guint i;
	for(i = 0; i < n_xwindows; i ++) {
		XdkWindow * window = xdk_display_lookup_window(
			priv->display,
			xwindows[i]);
		if(! window) {
			window = xdk_window_new();
			xdk_window_set_parent(window, self);
			xdk_window_set_foreign_peer(window, xwindows[i]);
		}
		
		windows = g_list_prepend(windows, window);
	}
	XFree(xwindows);

end:
	return g_list_reverse(windows);
}

void xdk_window_set_visual(XdkWindow * self, XdkVisual * visual)
{
	g_return_if_fail(self);
	g_return_if_fail(visual);
	
	XdkWindowPrivate * priv = self->priv;
	if(priv->visual) {
		g_object_unref(priv->visual);
	}
	
	priv->visual = g_object_ref(visual);
	
	if(! xdk_window_is_realized(self)) {
		return;
	}

	XSetWindowColormap(
		xdk_display_get_peer(priv->display),
		priv->peer,
		xdk_visual_to_colormap(visual));
}

XdkVisual * xdk_window_get_visual(XdkWindow * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->priv->visual;
}

void xdk_window_set_cursor(XdkWindow * self, Cursor cursor)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	XDefineCursor(xdk_display_get_peer(priv->display), priv->peer, cursor);
	xdk_display_flush(priv->display);
}

void xdk_window_configure(
	XdkWindow * self,
	guint value_mask,
	XWindowChanges * values)
{
	g_return_if_fail(self);
	g_return_if_fail(values);
	
	XdkWindowPrivate * priv = self->priv;
	if(CWX & value_mask) {
		priv->x = values->x;
	}
	else if(CWY & value_mask) {
		priv->y = values->y;
	}
	else if(CWWidth & value_mask) {
		priv->width = values->width;
	}
	else if(CWHeight & value_mask) {
		priv->height = values->height;
	}
	else if(CWBorderWidth & value_mask) {
		priv->border_width = values->border_width;
	}
	
	if(! xdk_window_is_realized(self)) {
		return;
	}
	
	XConfigureWindow(
		xdk_display_get_peer(priv->display),
		priv->peer,
		value_mask, values);
}

gboolean xdk_window_get_override_redirect(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return self->priv->override_redirect;
}

static gboolean contains_atom(
	XdkWindow * self,
	Atom * atoms, gint n_atoms,
	GQuark atom_name)
{
	Atom atom = xdk_display_atom_get(self->priv->display, atom_name);
	for(-- n_atoms; n_atoms >= 0; n_atoms --) {
		if(atoms[n_atoms] == atom) {
			return TRUE;
		}
	}
	
	return FALSE;
}

static void xdk_window_sync_net_wm_state(XdkWindow * self)
{
	g_return_if_fail(self);

	XdkWindowPrivate * priv = self->priv;
	Atom prop_type;
	gint prop_format;
	glong n_values, n_bytes_remain;
	Atom * values;
	guint result = XGetWindowProperty(
		xdk_display_get_peer(priv->display),
		priv->peer,
		xdk_display_atom_get(priv->display, XDK_ATOM_NET_WM_STATE),
		0, G_MAXINT,
		FALSE,
		AnyPropertyType,
		& prop_type, & prop_format,
		(gulong *) & n_values,
		(gulong *) & n_bytes_remain,
		(guchar **) & values);
	if(Success != result) {
		g_warning("Failed to get window property _NET_WM_STATE");
		return;
	}
	
	priv->maximized = contains_atom(self, values, n_values, XDK_ATOM_NET_WM_STATE_MAXIMIZED_VERT) ||
		contains_atom(self, values, n_values, XDK_ATOM_NET_WM_STATE_MAXIMIZED_HORZ);
	priv->visible = (TRUE != contains_atom(self, values, n_values, XDK_ATOM_NET_WM_STATE_HIDDEN));
	priv->fullscreen = contains_atom(self, values, n_values, XDK_ATOM_NET_WM_STATE_FULLSCREEN);
	priv->keep_above = contains_atom(self, values, n_values, XDK_ATOM_NET_WM_STATE_ABOVE);
	priv->keep_below = contains_atom(self, values, n_values, XDK_ATOM_NET_WM_STATE_BELOW);
		
	XFree(values);
}

void xdk_window_maximize(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(priv->maximized) {
		return;
	}
	
	priv->maximized = TRUE;
}

void xdk_window_unmaximize(XdkWindow * self)
{
	g_return_if_fail(self);

	XdkWindowPrivate * priv = self->priv;
	if(! priv->maximized) {
		return;
	}
	
	priv->maximized = FALSE;
}

gboolean xdk_window_is_maximized(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return self->priv->maximized;
}

void xdk_window_set_keep_above(XdkWindow * self, gboolean keep_above)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(priv->keep_above == keep_above) {
		return;
	}
	
	priv->keep_above = keep_above;
}

gboolean xdk_window_get_keep_above(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return self->priv->keep_above;
}

void xdk_window_set_keep_below(XdkWindow * self, gboolean keep_below)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(priv->keep_below == keep_below) {
		return;
	}
	
	priv->keep_below = keep_below;
}

gboolean xdk_window_get_keep_below(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return self->priv->keep_below;
}
