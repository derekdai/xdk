#include <gio/gio.h>
#include "xdk-display.h"
#include "xdk-window.h"
#include "xdk-window-manager.h"

#define DEFAULT_BACKGROUND_COLOR (0xff555753)

struct _XdkWindowManagerPrivate
{
	GList * screens;
	
	Cursor default_cursor;
	
	gulong background_color;
	
	gboolean has_default_cursor : 1;
};

enum
{
	PROP_N_SCREENS = 1,
	PROP_DEFAULT_CURSOR,
	PROP_HAS_DEFAULT_CURSOR,
	PROP_BACKGROUND_COLOR,
	PROP_LAST
};

enum
{
	SIGNAL_SCREEN_ADDED,
	SIGNAL_SCREEN_REMOVED,
	SIGNAL_WINDOW_ADDED,
	SIGNAL_WINDOW_SHOW,
	SIGNAL_WINDOW_HIDE,
	SIGNAL_WINDOW_RESTACK,
	SIGNAL_WINDOW_REMOVED,
	SIGNAL_LAST
};

static void xdk_window_manager_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec);

static void xdk_window_manager_get_property(
	GObject * object,
	guint property_id,
	GValue * value,
	GParamSpec * pspec);

static void xdk_window_manager_dispose(GObject * object);

static void xdk_window_manager_finalize(GObject * object);

static void xkd_window_manager_set_screen(
	XdkWindowManager * self,
	XdkScreen * screen);
	
static void xdk_window_manager_set_cursor_internal(
	XdkWindowManager * self,
	Cursor cursor);

static void xdk_window_manager_on_create(
	XdkWindowManager * self,
	XCreateWindowEvent * event,
	XdkWindow * root);
	
static void xdk_window_manager_on_configure_request(
	XdkWindowManager * self,
	XConfigureRequestEvent * event,
	XdkWindow * root);

static void xdk_window_manager_on_map_request(
	XdkWindowManager * self,
	XMapRequestEvent * event,
	XdkWindow * root);

static void xdk_window_manager_on_unmap_notify(
	XdkWindowManager * self,
	XUnmapEvent * event,
	XdkWindow * root);

static void xdk_window_manager_on_destroy_notify(
	XdkWindowManager * self,
	XDestroyWindowEvent * event,
	XdkWindow * root);
	
static XdkWindow * xdk_window_manager_lookup_window(
	XdkWindow * root,
	Window xwindow,
	gboolean create);
	
static gboolean xdk_window_manager_connect_signals(
	XdkWindowManager * self,
	XdkScreen * screen,
	GError ** error);

static void xdk_window_manager_disconnect_signals(
	XdkWindowManager * self,
	XdkScreen * screen);

static gboolean xdk_window_manager_add_screen_internal(
	XdkWindowManager * self,
	XdkScreen * screen,
	GError ** error);
	
static gboolean xdk_window_manager_remove_screen_internal(
	XdkWindowManager * self,
	XdkScreen * screen);

static void xdk_window_manager_on_screen_added(
	XdkWindowManager * self,
	XdkScreen * screen);
	
static void xdk_window_manager_on_screen_removed(
	XdkWindowManager * self,
	XdkScreen * screen);
	
G_DEFINE_TYPE(XdkWindowManager, xdk_window_manager, G_TYPE_OBJECT);

static GParamSpec * properties[PROP_LAST];

static guint signals[SIGNAL_LAST];

static GQuark quark_window_manager;
static GQuark quark_hide;
static GQuark quark_above;
static GQuark quark_below;
static GQuark quark_origin_geometry;

GQuark xdk_window_manager_error()
{
	static volatile GQuark error_domain = 0;
	if(G_UNLIKELY(! error_domain)) {
		error_domain = g_quark_from_static_string("XdkWindowManagerError");
	}
	
	return error_domain;
}

static void xdk_window_manager_class_init(XdkWindowManagerClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->set_property = xdk_window_manager_set_property;
	gobject_class->get_property = xdk_window_manager_get_property;
	gobject_class->dispose = xdk_window_manager_dispose;
	gobject_class->finalize = xdk_window_manager_finalize;
	
	clazz->screen_added = xdk_window_manager_on_screen_added;
	clazz->screen_removed = xdk_window_manager_on_screen_removed;
	
	properties[PROP_N_SCREENS] = g_param_spec_int(
			"screen", "", "",
			0, G_MAXINT, 0,
			G_PARAM_READABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
	g_object_class_install_property(
		gobject_class, 
		PROP_N_SCREENS, 
		properties[PROP_N_SCREENS]);
	
	properties[PROP_DEFAULT_CURSOR] = g_param_spec_ulong(
			"default-cursor", "", "",
			0, G_MAXULONG, None,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
	g_object_class_install_property(
		gobject_class,
		PROP_DEFAULT_CURSOR,
		properties[PROP_DEFAULT_CURSOR]);
			
	properties[PROP_HAS_DEFAULT_CURSOR] = g_param_spec_boolean(
			"has-default-cursor", "", "",
			FALSE,
			G_PARAM_READABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
	g_object_class_install_property(
		gobject_class,
		PROP_HAS_DEFAULT_CURSOR,
		properties[PROP_HAS_DEFAULT_CURSOR]);
		
	properties[PROP_BACKGROUND_COLOR] = g_param_spec_ulong(
			"background-color", "", "",
			0, G_MAXULONG, DEFAULT_BACKGROUND_COLOR,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
	g_object_class_install_property(
		gobject_class,
		PROP_BACKGROUND_COLOR,
		properties[PROP_BACKGROUND_COLOR]);
			
	GType type = XDK_TYPE_WINDOW_MANAGER;
	signals[SIGNAL_WINDOW_ADDED] = g_signal_new(
		"window-added",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT,
		G_TYPE_NONE, 1, XDK_TYPE_WINDOW);

	signals[SIGNAL_WINDOW_SHOW] = g_signal_new(
		"window-show",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT,
		G_TYPE_NONE, 1, XDK_TYPE_WINDOW);
	
	signals[SIGNAL_WINDOW_HIDE] = g_signal_new(
		"window-hide",
		type,
		G_SIGNAL_RUN_FIRST,
		0,
		NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT,
		G_TYPE_NONE, 1, XDK_TYPE_WINDOW);
			
	signals[SIGNAL_WINDOW_REMOVED] = g_signal_new(
		"window-removed",
		type,
		G_SIGNAL_RUN_LAST,
		0,
		NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT,
		G_TYPE_NONE, 1, XDK_TYPE_WINDOW);
			
	signals[SIGNAL_SCREEN_ADDED] = g_signal_new(
		"screen-added",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowManagerClass, screen_added),
		NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT,
		G_TYPE_NONE, 1, XDK_TYPE_SCREEN);
			
	signals[SIGNAL_SCREEN_REMOVED] = g_signal_new(
		"screen-removed",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowManagerClass, screen_removed),
		NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT,
		G_TYPE_NONE, 1, XDK_TYPE_SCREEN);
		
	quark_window_manager = g_quark_from_static_string("XdkWindowManager");
	quark_hide = g_quark_from_static_string("XdkWindowManagerHide");
	quark_above = g_quark_from_static_string("XdkWindowManagerAbove");
	quark_below = g_quark_from_static_string("XdkWindowManagerBelow");
	quark_origin_geometry = g_quark_from_static_string("XdkWindowManagerOriginGeometry");

	g_type_class_add_private(clazz, sizeof(XdkWindowManagerPrivate));
}

static void xdk_window_manager_init(XdkWindowManager * self)
{
	XdkWindowManagerPrivate * priv = G_TYPE_INSTANCE_GET_PRIVATE(
		self,
		XDK_TYPE_WINDOW_MANAGER,
		XdkWindowManagerPrivate);
	self->priv = priv;
		
	priv->has_default_cursor = FALSE;
	priv->default_cursor = None;
	priv->background_color = DEFAULT_BACKGROUND_COLOR;
}

XdkWindowManager * xdk_window_manager_new()
{
	return g_object_new(XDK_TYPE_WINDOW_MANAGER, NULL);
}

static XdkWindow * xdk_window_manager_lookup_window(
	XdkWindow * root,
	Window xwindow,
	gboolean create)
{
	XdkWindow * window = xdk_display_lookup_window(
		xdk_window_get_display(root),
		xwindow);
	if(window || ! create) {
		goto end;
	}
	
	window = xdk_window_new();
	xdk_window_set_foreign_peer(window, xwindow);
	
end:
	return window;
}

static void xdk_window_manager_on_create(
	XdkWindowManager * self,
	XCreateWindowEvent * event,
	XdkWindow * root)
{
	XdkWindow * window = xdk_window_manager_lookup_window(
		root,
		event->window,
		TRUE);
		
	g_signal_emit(self, signals[SIGNAL_WINDOW_ADDED], 0, window);
}

static void xdk_window_manager_on_configure_request(
	XdkWindowManager * self,
	XConfigureRequestEvent * event,
	XdkWindow * root)
{
	g_message("xdk_window_manager_on_configure_request");

	XdkWindow * window = xdk_window_manager_lookup_window(
		root,
		event->window,
		FALSE);
	XWindowChanges change = {
		.x = event->x,
		.y = event->y,
		.width = event->width,
		.height = event->height,
		.border_width = event->border_width,
		.sibling = event->above,
		.stack_mode = event->detail
	};
	xdk_window_configure(window, event->value_mask, & change);
}

static void xdk_window_manager_on_map_request(
	XdkWindowManager * self,
	XMapRequestEvent * event,
	XdkWindow * root)
{
	g_message("xdk_window_manager_on_map_request");

	XdkWindow * window = xdk_window_manager_lookup_window(
		root,
		event->window,
		FALSE);

	xdk_window_map(window);
}

static void xdk_window_manager_on_map_notify(
	XdkWindowManager * self,
	XMapEvent * event,
	XdkWindow * root)
{
	g_message("xdk_window_manager_on_map_notify");

	XdkWindow * window = xdk_window_manager_lookup_window(
		root,
		event->window,
		FALSE);

	g_signal_emit(self, signals[SIGNAL_WINDOW_SHOW], 0, window);
}

static void xdk_window_manager_on_unmap_notify(
	XdkWindowManager * self,
	XUnmapEvent * event,
	XdkWindow * root)
{
	g_message("xdk_window_manager_on_unmap_notify");

	XdkWindow * window = xdk_window_manager_lookup_window(
		root,
		event->window,
		FALSE);

	g_signal_emit(self, signals[SIGNAL_WINDOW_HIDE], 0, window);
}

static void xdk_window_manager_on_destroy_notify(
	XdkWindowManager * self,
	XDestroyWindowEvent * event,
	XdkWindow * root)
{
	g_message("xdk_window_manager_on_destroy_notify: %lu", event->window);
	
	XdkWindow * window = xdk_window_manager_lookup_window(
		root,
		event->window,
		FALSE);
	
	if(! window) {
		return;
	}
	
	g_signal_emit(self, signals[SIGNAL_WINDOW_REMOVED], 0, window);

	xdk_window_destroy(window);
}

static void xdk_window_manager_dispose(GObject * object)
{
	XdkWindowManager * self = XDK_WINDOW_MANAGER(object);
	XdkWindowManagerPrivate * priv = self->priv;
	
	//xdk_window_manager_disconnect_signals(self);
}

static void xdk_window_manager_finalize(GObject * object)
{
	G_OBJECT_CLASS(xdk_window_manager_parent_class)->finalize(object);
}

static void xdk_window_manager_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec)
{
	XdkWindowManager * self = XDK_WINDOW_MANAGER(object);
	XdkWindowManagerPrivate * priv = self->priv;
	
	switch(property_id) {
	case PROP_DEFAULT_CURSOR:
		xdk_window_manager_set_default_cursor(
			self,
			g_value_get_ulong(value));
		break;
	case PROP_BACKGROUND_COLOR:
		xdk_window_manager_set_background_color(
			self,
			g_value_get_ulong(value));
	default:
		g_return_if_reached();
	}
}

static void xdk_window_manager_get_property(
	GObject * object,
	guint property_id,
	GValue * value,
	GParamSpec * pspec)
{
	XdkWindowManager * self = XDK_WINDOW_MANAGER(object);
	XdkWindowManagerPrivate * priv = self->priv;
	
	switch(property_id) {
	case PROP_N_SCREENS:
		g_value_set_int(value, g_list_length(priv->screens));
		break;
	case PROP_DEFAULT_CURSOR:
		xdk_window_manager_set_default_cursor(
			self,
			g_value_get_ulong(value));
		break;
	default:
		g_return_if_reached();
	}
}

void xdk_window_manager_unset_default_cursor(XdkWindowManager * self)
{
	g_return_if_fail(self);
	
	XdkWindowManagerPrivate * priv = self->priv;
	priv->default_cursor = None;
	priv->has_default_cursor = FALSE;
	//xdk_window_set_cursor(priv->root, None);
}

static void xdk_window_manager_set_cursor_internal(
	XdkWindowManager * self,
	Cursor cursor)
{
	XdkWindowManagerPrivate * priv = self->priv;
	priv->default_cursor = cursor;
	priv->has_default_cursor = TRUE;
	
	/*
	if(priv->root) {
		xdk_window_set_cursor(priv->root, cursor);
	}
	*/
}

void xdk_window_manager_set_default_cursor(
	XdkWindowManager * self,
	Cursor cursor)
{
	g_return_if_fail(self);
	
	if(cursor == self->priv->default_cursor) {
		return;
	}
	
	xdk_window_manager_set_cursor_internal(self, cursor);
	
	g_object_notify_by_pspec(
		(GObject *) self,
		properties[PROP_DEFAULT_CURSOR]);
}

Cursor xdk_window_manager_get_default_cursor(XdkWindowManager * self)
{
	g_return_val_if_fail(self, None);
	
	return self->priv->default_cursor;
}

static gboolean xdk_window_manager_connect_signals(
	XdkWindowManager * self,
	XdkScreen * screen,
	GError ** error)
{
	XdkWindowManagerPrivate * priv = self->priv;

	XdkDisplay * display = xdk_screen_get_display(screen);
	XdkWindow * root = xdk_screen_get_root_window(screen);
	
	xdk_trap_error();
	xdk_window_select_input(
		root,
		XDK_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
		XDK_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
		XDK_EVENT_MASK_STRUCTURE_NOTIFY);
	xdk_display_flush(display);
	
	if(xdk_untrap_error(NULL)) {
		g_set_error(
			error,
			XDK_WINDOW_MANAGER_ERROR,
			XDK_WINDOW_MANAGER_ERROR_WM_EXISTS,
			"Another window manager exists");
		
		return FALSE;
	}
	
	g_signal_connect_swapped(
		root,
		"create-notify",
		G_CALLBACK(xdk_window_manager_on_create), self);
		
	g_signal_connect_swapped(
		root,
		"configure-request",
		G_CALLBACK(xdk_window_manager_on_configure_request), self);
		
	g_signal_connect_swapped(
		root,
		"map-request",
		G_CALLBACK(xdk_window_manager_on_map_request), self);
		
	g_signal_connect_swapped(
		root,
		"map-notify",
		G_CALLBACK(xdk_window_manager_on_map_notify), self);
		
	g_signal_connect_swapped(
		root,
		"unmap-notify",
		G_CALLBACK(xdk_window_manager_on_unmap_notify), self);
		
	g_signal_connect_swapped(
		root,
		"destroy-notify",
		G_CALLBACK(xdk_window_manager_on_destroy_notify), self);

	return TRUE;
}

gboolean xdk_window_manager_has_screen(
	XdkWindowManager * self,
	XdkScreen * screen)
{
	return NULL != g_list_find(self->priv->screens, screen);
}

static gboolean xdk_window_manager_add_screen_internal(
	XdkWindowManager * self,
	XdkScreen * screen,
	GError ** error)
{
	XdkWindowManagerPrivate * priv = self->priv;
	if(g_list_find(priv->screens, screen)) {
		return TRUE;
	}
	
	if(! xdk_window_manager_connect_signals(self, screen, error)) {
		return FALSE;
	}

	priv->screens = g_list_prepend(priv->screens, screen);
	g_object_set_qdata((GObject *) screen, quark_window_manager, self);
	
	return TRUE;
}

gboolean xdk_window_manager_add_screen(
	XdkWindowManager * self,
	XdkScreen * screen,
	GError ** error)
{
	g_return_val_if_fail(self, FALSE);
	g_return_val_if_fail(! error || ! * error, FALSE);
	
	if(! screen) {
		screen = xdk_get_default_screen();
	}
	
	XdkDisplay * display = xdk_screen_get_display(screen);
	xdk_display_grab_server(display);
	gboolean result = xdk_window_manager_add_screen_internal(
		self,
		screen,
		error);
	xdk_display_ungrab_server(display);
	
	if(result) {
		g_signal_emit(self, signals[SIGNAL_SCREEN_ADDED], 0, screen);
	}

	return result;
}

gboolean xdk_window_manager_add_all_screens(
	XdkWindowManager * self,
	XdkDisplay * display,
	GError ** error)
{
	g_return_val_if_fail(self, FALSE);
	g_return_val_if_fail(! error || ! * error, FALSE);
	
	if(! display) {
		display = xdk_display_get_default();
	}
	
	xdk_display_grab_server(display);
	
	GError * e = NULL;
	GList * screens = xdk_display_list_screens(display);
	GList * node = screens;
	for(; node; node = g_list_next(node)) {
		if(! xdk_window_manager_add_screen_internal(self, node->data, & e)) {
			break;
		}
	}
	
	xdk_display_ungrab_server(display);
	
	if(e) {
		g_propagate_error(error, e);
		for(; node; node = g_list_previous(node)) {
			xdk_window_manager_remove_screen_internal(self, node->data);
		}
	}
	else {
		for(node = screens; node; node = g_list_next(node)) {
			g_signal_emit(self, signals[SIGNAL_SCREEN_ADDED], 0, node->data);
		}
	}
	
	g_list_free(screens);
	
	return NULL == e;
}

static void xdk_window_manager_disconnect_signals(
	XdkWindowManager * self,
	XdkScreen * screen)
{
	XdkDisplay * display = xdk_screen_get_display(screen);
	XdkWindow * root = xdk_screen_get_root_window(screen);
	
	xdk_trap_error();
	xdk_window_select_input(root, 0);
	xdk_display_flush(display);
	xdk_untrap_error(NULL);

	g_signal_handlers_disconnect_by_data(root, screen);
}

static gboolean xdk_window_manager_remove_screen_internal(
	XdkWindowManager * self,
	XdkScreen * screen)
{
	GList * node = g_list_find(self->priv->screens, screen);
	if(! node) {
		return FALSE;
	}

	XdkWindowManagerPrivate * priv = self->priv;
	priv->screens = g_list_remove_link(priv->screens, node);
	g_list_free(node);

	g_object_set_qdata((GObject *) screen, quark_window_manager, NULL);
	
	return TRUE;
}
	
void xdk_window_manager_remove_screen(XdkWindowManager * self, XdkScreen * screen)
{
	g_return_if_fail(self);
	g_return_if_fail(screen);
	
	if(! xdk_window_manager_remove_screen_internal(self, screen)) {
		return;
	}
	
	g_signal_emit(self, signals[SIGNAL_SCREEN_REMOVED], 0, screen);
}

void xdk_window_manager_remove_all_screens(XdkWindowManager * self)
{
	g_return_if_fail(self);
	
	XdkWindowManagerPrivate * priv = self->priv;
	while(priv->screens) {
		xdk_window_manager_remove_screen(self, priv->screens->data);
	}
}

GList * xdk_window_manager_list_screens(XdkWindowManager * self)
{
	g_return_val_if_fail(self, NULL);
	
	GList * screens = NULL;
	GList * node = self->priv->screens;
	for(; node; node = g_list_next(node)) {
		screens = g_list_append(screens, node->data);
	}
	
	return screens;
}

gint xdk_window_manager_n_screens(XdkWindowManager * self)
{
	g_return_val_if_fail(self, 0);
	
	return g_list_length(self->priv->screens);
}

static void xdk_window_maximize_window(
	XdkWindowManager * self,
	XdkWindow * window)
{
	if(g_object_get_qdata((GObject *) window, quark_origin_geometry)) {
		return;
	}
	
	XdkGeometry * orig_geometry = xdk_geometry_new();
	xdk_window_get_geometry(
		window,
		& orig_geometry->x, & orig_geometry->y,
		& orig_geometry->width, & orig_geometry->height);
	g_object_set_qdata((GObject *) window, quark_origin_geometry, orig_geometry);
		
	xdk_window_set_geometry(window, 0, 0, -1, -1);
}

static void xdk_window_unmaximize_window(
	XdkWindowManager * self,
	XdkWindow * window)
{
	XdkGeometry * origin_geometry = g_object_get_qdata(
		(GObject *) window,
		quark_origin_geometry);
	if(! origin_geometry) {
		return;
	}
}

static void xdk_window_manager_add_window(
	XdkWindowManager * self,
	XdkWindow * window)
{
	XdkScreen * screen = xdk_window_get_screen(window);
	GQuark quark = 0;
	if(xdk_window_get_keep_above(window)) {
		quark = quark_above;
	}
	else if(xdk_window_get_keep_below(window)) {
		quark = quark_below;
	}

	if(quark) {
		GList * windows = g_object_get_qdata((GObject *) screen, quark);
		windows = g_list_prepend(windows, window);
		g_object_set_qdata((GObject *) screen, quark, windows);
	}
	
	if(xdk_window_is_maximized(window)) {
		xdk_window_maximize_window(self, window);
	}
	
	g_signal_emit(self, signals[SIGNAL_WINDOW_ADDED], 0, window);
}

static void xdk_window_manager_on_screen_added(
	XdkWindowManager * self,
	XdkScreen * screen)
{
	xdk_window_set_background_color(
		xdk_screen_get_root_window(screen),
		self->priv->background_color);
	
	XdkWindow * root = xdk_screen_get_root_window(screen);
	
	GList * windows = xdk_window_query_tree(root);
	GList * node = windows;
	for(; node; node = g_list_next(node)) {
		xdk_window_manager_add_window(self, node->data);
	}
}
	
static void xdk_window_manager_on_screen_removed(
	XdkWindowManager * self,
	XdkScreen * screen)
{
}

static void xdk_window_manager_set_screen_background_color(
	XdkScreen * screen,
	gulong background_color)
{
	xdk_window_set_background_color(
		xdk_screen_get_root_window(screen),
		background_color);
}

void xdk_window_manager_set_background_color(
	XdkWindowManager * self,
	gulong background_color)
{
	g_return_if_fail(self);
	
	XdkWindowManagerPrivate * priv = self->priv;
	if(priv->background_color == background_color) {
		return;
	}
	
	priv->background_color = background_color;
	
	g_list_foreach(
		priv->screens,
		(GFunc) xdk_window_manager_set_screen_background_color,
		GUINT_TO_POINTER(priv->background_color));
	
	g_object_notify_by_pspec(
		(GObject *) self,
		properties[PROP_BACKGROUND_COLOR]);
}

gulong xdk_window_manager_get_background_color(XdkWindowManager * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->background_color;
}
