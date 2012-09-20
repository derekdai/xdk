#include <gio/gio.h>
#include "xdk-display.h"
#include "xdk-window.h"
#include "xdk-window-manager.h"

struct _XdkWindowManagerPrivate
{
	XdkDisplay * display;
	
	XdkScreen * screen;
	
	XdkWindow * root;
	
	Cursor default_cursor;
	
	gboolean has_default_cursor : 1;
};

enum
{
	PROP_SCREEN = 1,
	PROP_DEFAULT_CURSOR,
	PROP_HAS_DEFAULT_CURSOR,
	PROP_LAST
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

static void xdk_window_manager_initiable_iface_init(GInitableIface * iface);

static gboolean xdk_window_manager_initiable_init(
	GInitable * initable,
	GCancellable * cancellable,
	GError ** error);
	
static void xdk_window_manager_dispose(GObject * object);

static void xdk_window_manager_finalize(GObject * object);

static void xkd_window_manager_set_screen(
	XdkWindowManager * self,
	XdkScreen * screen);
	
static void xdk_window_manager_set_cursor_internal(
	XdkWindowManager * self,
	Cursor cursor);
	
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
	XdkWindowManager * self,
	Window xwindow,
	gboolean create);
	
static void xdk_window_manager_connect_signals(XdkWindowManager * self);

static void xdk_window_manager_disconnect_signals(XdkWindowManager * self);
	
G_DEFINE_TYPE_WITH_CODE(XdkWindowManager, xdk_window_manager, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE, xdk_window_manager_initiable_iface_init));

static GParamSpec * properties[PROP_LAST];

GQuark xdk_window_manager_error()
{
	static volatile GQuark error_domain = 0;
	if(G_UNLIKELY(! error_domain)) {
		error_domain = g_quark_from_static_string("XdkWindowManagerError");
	}
	
	return error_domain;
}

static void xdk_window_manager_initiable_iface_init(GInitableIface * iface)
{
	iface->init = xdk_window_manager_initiable_init;
}

static void xdk_window_manager_class_init(XdkWindowManagerClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->set_property = xdk_window_manager_set_property;
	gobject_class->get_property = xdk_window_manager_get_property;
	gobject_class->dispose = xdk_window_manager_dispose;
	gobject_class->finalize = xdk_window_manager_finalize;
	
	properties[PROP_SCREEN] = g_param_spec_object(
			"screen", "", "",
			XDK_TYPE_SCREEN,
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
	g_object_class_install_property(
		gobject_class, 
		PROP_SCREEN, 
		properties[PROP_SCREEN]);
	
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
}

static XdkWindow * xdk_window_manager_lookup_window(
	XdkWindowManager * self,
	Window xwindow,
	gboolean create)
{
	XdkWindowManagerPrivate * priv = self->priv;
	XdkWindow * window = xdk_display_lookup_window(
		priv->display,
		xwindow);
	if(window || ! create) {
		goto end;
	}
	
	window = xdk_window_new();
	xdk_window_set_foreign_peer(window, xwindow);
	
end:
	return window;
}

static void xdk_window_manager_on_configure_request(
	XdkWindowManager * self,
	XConfigureRequestEvent * event,
	XdkWindow * root)
{
	XdkWindow * window = xdk_window_manager_lookup_window(
		self,
		event->window,
		TRUE);
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
	XdkWindowManagerPrivate * priv = self->priv;
	XdkWindow * window = xdk_window_manager_lookup_window(
		self,
		event->window,
		TRUE);
	xdk_window_map(window);
}

static void xdk_window_manager_on_unmap_notify(
	XdkWindowManager * self,
	XUnmapEvent * event,
	XdkWindow * root)
{
}

static void xdk_window_manager_on_destroy_notify(
	XdkWindowManager * self,
	XDestroyWindowEvent * event,
	XdkWindow * root)
{
	XdkWindow * window = xdk_window_manager_lookup_window(
		self,
		event->window,
		FALSE);
	if(window) {
		xdk_window_destroy(window);
		g_object_unref(window);
	}
}

static gboolean xdk_window_manager_on_event(
	XdkDisplay * display,
	XDestroyWindowEvent * event,
	XdkWindowManager * self)
{
	gboolean result = FALSE;
	XdkWindowManagerPrivate * priv = self->priv;
	
	switch(event->type) {
	case XDK_EVENT_DESTROY:
		g_signal_emit_by_name(priv->root, "destroy-notify", event);
		break;
	}
	
	return result;
}

static void xdk_window_manager_connect_signals(XdkWindowManager * self)
{
	XdkWindowManagerPrivate * priv = self->priv;
	g_signal_connect_swapped(
		priv->root,
		"configure-request",
		G_CALLBACK(xdk_window_manager_on_configure_request), self);
		
	g_signal_connect_swapped(
		priv->root,
		"map-request",
		G_CALLBACK(xdk_window_manager_on_map_request), self);
		
	g_signal_connect_swapped(
		priv->root,
		"unmap-notify",
		G_CALLBACK(xdk_window_manager_on_unmap_notify), self);
		
	g_signal_connect_swapped(
		priv->root,
		"destroy-notify",
		G_CALLBACK(xdk_window_manager_on_destroy_notify), self);
		
	xdk_display_add_event_filter(
		priv->display,
		(XdkEventFilter) xdk_window_manager_on_event,
		self);
}

static void xdk_window_manager_disconnect_signals(XdkWindowManager * self)
{
	XdkWindowManagerPrivate * priv = self->priv;
	xdk_display_remove_event_filter(
		priv->display,
		(XdkEventFilter) xdk_window_manager_on_event,
		self);

	g_signal_handlers_disconnect_by_data(priv->root, self);
}

static gboolean xdk_window_manager_initiable_init(
	GInitable * initable,
	GCancellable * cancellable,
	GError ** error)
{
	XdkWindowManager * self = XDK_WINDOW_MANAGER(initable);
	XdkWindowManagerPrivate * priv = self->priv;
	
	// redirect event and check if another window manager is running
	xdk_trap_error();
	xdk_window_select_input(
		priv->root,
		XDK_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
		XDK_EVENT_MASK_SUBSTRUCTURE_NOTIFY);
	xdk_display_flush(priv->display);
	
	if(xdk_untrap_error(NULL)) {
		g_set_error(
			error,
			XDK_WINDOW_MANAGER_ERROR,
			XDK_WINDOW_MANAGER_ERROR_WM_EXISTS,
			"Another window manager exists");
		
		return FALSE;
	}
	
	// assign a default cursor
	if(priv->has_default_cursor) {
		xdk_window_manager_set_cursor_internal(self, priv->default_cursor);
	}
	
	xdk_window_manager_connect_signals(self);
	
	return TRUE;
}

static void xdk_window_manager_dispose(GObject * object)
{
	XdkWindowManager * self = XDK_WINDOW_MANAGER(object);
	XdkWindowManagerPrivate * priv = self->priv;
	
	xdk_window_manager_disconnect_signals(self);
}

static void xdk_window_manager_finalize(GObject * object)
{
	G_OBJECT_CLASS(xdk_window_manager_parent_class)->finalize(object);
}

XdkWindowManager * xdk_window_manager_new(XdkScreen * screen, GError ** error)
{
	return g_initable_new(XDK_TYPE_WINDOW_MANAGER, NULL, error, "screen", screen, NULL);
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
	case PROP_SCREEN:
		xkd_window_manager_set_screen(
			self,
			XDK_SCREEN(g_value_get_object(value)));
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

static void xkd_window_manager_set_screen(
	XdkWindowManager * self,
	XdkScreen * screen)
{
	g_return_if_fail(self);
	g_return_if_fail(screen);
	
	XdkWindowManagerPrivate * priv = self->priv;
	
	priv->screen = screen;
	priv->display = xdk_screen_get_display(priv->screen);
	priv->root = xdk_screen_get_root_window(priv->screen);
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
	case PROP_DEFAULT_CURSOR:
		g_value_set_ulong(value, xdk_window_manager_get_default_cursor(self));
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
	xdk_window_set_cursor(priv->root, None);
}

static void xdk_window_manager_set_cursor_internal(
	XdkWindowManager * self,
	Cursor cursor)
{
	XdkWindowManagerPrivate * priv = self->priv;
	priv->default_cursor = cursor;
	priv->has_default_cursor = TRUE;
	
	if(priv->root) {
		xdk_window_set_cursor(priv->root, cursor);
	}
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
