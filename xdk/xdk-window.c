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
	
	guint width;
	
	guint height;
	
	XdkVisual * visual;
	
	gulong background_color;
	
	gulong event_mask;
	
	XdkGravity gravity;
	
	gboolean mapped : 1;
	
	gboolean visible : 1;
	
	gboolean own_peer : 1;
	
	gboolean destroyed : 1;
};

enum {
	XDK_WINDOW_DELETE_EVENT = XDK_EVENT_LAST,
	XDK_WINDOW_DESTROY,
	SIGNAL_MAX,
};

static void xdk_window_dispose(GObject * object);

static void xdk_window_finalize(GObject * object);

static void xdk_window_default_realize(XdkWindow * self);

static void xdk_window_default_unrealize(XdkWindow * self);

static void xdk_window_default_map(XdkWindow * self);

static void xdk_window_default_unmap(XdkWindow * self);

static void xdk_window_handle_delete_event(XdkWindow * self, XEvent * event);

G_DEFINE_TYPE(XdkWindow, xdk_window, G_TYPE_OBJECT);

static guint signals[SIGNAL_MAX] = { 0, };

static void xdk_window_class_init(XdkWindowClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->dispose = xdk_window_dispose;
	gobject_class->finalize = xdk_window_finalize;
	
	clazz->realize = xdk_window_default_realize;
	clazz->unrealize = xdk_window_default_unrealize;
	clazz->map = xdk_window_default_map;
	clazz->unmap = xdk_window_default_unmap;
	clazz->delete_event = xdk_window_handle_delete_event;
	
	/*
	signals[XDK_EVENT_CREATE] = g_signal_new(
		"realize",
		type,
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkWindowClass, realize),
		NULL, NULL,
		g_cclosure_marshal_VOID__BOXED,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_MAP] = g_signal_new(
		"map",
		type,
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkWindowClass, map),
		NULL, NULL,
		g_cclosure_marshal_VOID__BOXED,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_EVENT_UNMAP] = g_signal_new(
		"unmap",
		type,
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkWindowClass, unmap),
		NULL, NULL,
		g_cclosure_marshal_VOID__BOXED,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
		*/

	GType type = G_TYPE_FROM_CLASS(clazz);
	signals[XDK_WINDOW_DELETE_EVENT] = g_signal_new(
		"delete",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, delete_event),
		NULL, NULL,
		g_cclosure_marshal_VOID__BOXED,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
	
	signals[XDK_WINDOW_DESTROY] = g_signal_new(
		"destroy",
		type,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(XdkWindowClass, destroy),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);
	
	/*
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
	case :
	case :
	case :
	case :
	case XDK_EVENT_MAP_REQUEST:
	case XDK_EVENT_REPARENT:
	case XDK_EVENT_CONFIGURE:
	case XDK_EVENT_CONFIGURE_REQUEST:
	case XDK_EVENT_GRAVITY:
	case XDK_EVENT_RESIZE_REQUEST:
	case XDK_EVENT_CIRCULATE:
	case XDK_EVENT_CIRCULATE_REQUEST:
	case XDK_EVENT_PROPERTY:
	case XDK_EVENT_SELECTION_CLEAR:
	case XDK_EVENT_SELECTION_REQUEST:
	case XDK_EVENT_SELECTION:
	case XDK_EVENT_COLORMAP:
	case XDK_EVENT_CLIENT_MESSAGE:
	case XDK_EVENT_MAPPING:
	case XDK_EVENT_GENERIC:
	*/
		
	g_type_class_add_private(clazz, sizeof(XdkWindowPrivate));
}

static void xdk_window_init(XdkWindow * self)
{
	XdkWindowPrivate * priv = XDK_WINDOW_GET_PRIVATE(self);
	self->priv = priv;
	
	priv->display = xdk_display_get_default();
	priv->screen = xdk_display_get_default_screen(priv->display);
	priv->peer = None;
	priv->background_color = 0xffffffff;
	priv->width = 1;
	priv->height = 1;
	priv->gravity = XDK_GRAVITY_CENTER;
	priv->event_mask = XDK_EVENT_MASK_STRUCTURE_NOTIFY |
		XDK_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
}

static void xdk_window_dispose(GObject * object)
{
	XdkWindowPrivate * priv = XDK_WINDOW(object)->priv;
}

static void xdk_window_finalize(GObject * object)
{
	XdkWindow * self = XDK_WINDOW(object);
	
	xdk_window_destroy(self);
	
	G_OBJECT_CLASS(xdk_window_parent_class)->finalize(object);
}

XdkWindow * xdk_window_new()
{
	return XDK_WINDOW(g_object_new(XDK_TYPE_WINDOW, NULL));
}

static void _xdk_window_set_peer(XdkWindow * self, Window peer, gboolean own_peer)
{
	g_return_if_fail(self);

	XdkWindowPrivate * priv = self->priv;
	priv->peer = peer;
	priv->own_peer = own_peer;
	
	if(None != peer) {
		xdk_display_add_window(priv->display, self);
	}
}

void xdk_window_set_foreign_peer(XdkWindow * self, Window peer)
{
	_xdk_window_set_peer(self, peer, FALSE);
}

void xdk_window_take_peer(XdkWindow * self, Window peer)
{
	_xdk_window_set_peer(self, peer, TRUE);
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
	
	XdkWindowPrivate * priv = self->priv;
	return XCreateWindow(
		xdk_display_get_peer(priv->display),
		xdk_window_get_peer(parent),
		priv->x, priv->y,
		priv->width, priv->height,
		0,									/* border width */
		CopyFromParent,						/* color depth */
		window_type,
		CopyFromParent,						/* visual */
		attribute_mask,
		attributes);
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
	XdkWindow * parent = priv->parent
		? priv->parent 
		: xdk_get_default_root_window();
	XSetWindowAttributes attributes = {
		.event_mask = priv->event_mask,
		.background_pixel = priv->background_color,
		.win_gravity = priv->gravity,
	};
	Window peer = xdk_window_create_window(
		self,
		parent,
		XDK_WINDOW_CLASSES_INPUT_OUTPUT,
		XDK_ATTR_MASK_BACKGROUND_COLOR | XDK_ATTR_MASK_WIN_GRAVITY |
			XDK_ATTR_MASK_EVENT_MASK,
		& attributes);
	xdk_window_take_peer(self, peer);
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

static void xdk_window_default_map(XdkWindow * self)
{
	XdkWindowPrivate * priv = self->priv;
	XMapWindow(xdk_display_get_peer(priv->display), priv->peer);
}

static void xdk_window_default_unmap(XdkWindow * self)
{
	XdkWindowPrivate * priv = self->priv;
	XUnmapWindow(xdk_display_get_peer(priv->display), priv->peer);
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
	
	self->priv->x = x;
	self->priv->y = y;
}

void xdk_window_get_size(XdkWindow * self, int * width, int * height)
{
	g_return_if_fail(self);

	if(width) {
		* width = self->priv->width;
	}
	
	if(height) {
		* height = self->priv->height;
	}
}

void xdk_window_set_size(XdkWindow * self, int width, int height)
{
	g_return_if_fail(self);
	
	self->priv->width = width;
	self->priv->height = height;
}

void xdk_window_map(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindowPrivate * priv = self->priv;
	if(xdk_window_is_mapped(self)) {
		return;
	}
	
	xdk_window_realize(self);
	XDK_WINDOW_GET_CLASS(self)->map(self);
	priv->mapped = TRUE;
}

gboolean xdk_window_is_mapped(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	XdkWindowPrivate * priv = self->priv;
	return ! priv->destroyed && None != priv->peer && priv->mapped;
}

void xdk_window_unmap(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! xdk_window_is_mapped(self)) {
		return;
	}
	
	XDK_WINDOW_GET_CLASS(self)->unmap(self);
	self->priv->mapped = FALSE;
}

void xdk_window_show(XdkWindow * self)
{
	g_return_if_fail(self);
	
	xdk_window_realize(self);
	xdk_window_map(self);
}

void xdk_window_hide(XdkWindow * self)
{
	xdk_window_unmap(self);
}

gboolean xdk_window_is_destroyed(XdkWindow * self)
{
	g_return_val_if_fail(self, TRUE);
	
	return self->priv->destroyed;
}

void xdk_window_destroy(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! self->priv->own_peer) {
		return;
	}
	
	xdk_window_unmap(self);
	xdk_window_unrealize(self);
	
	self->priv->destroyed = TRUE;
}

Atom * xdk_window_list_properties(XdkWindow * self, int * n_props)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(n_props, NULL);
	
	xdk_window_realize(self);
	
	return XListProperties(
		xdk_display_get_peer(self->priv->display),
		self->priv->peer,
		n_props);
}

void xdk_window_dispatch_event(XdkWindow * self, XEvent * event)
{
	switch(event->type) {
	case XDK_EVENT_KEY_PRESS:
		break;
	case XDK_EVENT_KEY_RELEASE:
		break;
	case XDK_EVENT_BUTTON_PRESS:
		break;
	case XDK_EVENT_BUTTON_RELEASE:
		break;
	case XDK_EVENT_MOTION:
		break;
	case XDK_EVENT_ENTER:
		break;
	case XDK_EVENT_LEAVE:
		break;
	case XDK_EVENT_FOCUS_IN:
		break;
	case XDK_EVENT_FOCUS_OUT:
		break;
	case XDK_EVENT_KEYMAP:
		break;
	case XDK_EVENT_EXPOSE:
		break;
	case XDK_EVENT_GRAPHICS_EXPOSE:
		break;
	case XDK_EVENT_NO_EXPOSE:
		break;
	case XDK_EVENT_VISIBILITY:
		break;
	case XDK_EVENT_CREATE:
		break;
	case XDK_EVENT_DESTROY:
		g_signal_emit(self, signals[XDK_EVENT_DESTROY], 0, event);
		break;
	case XDK_EVENT_UNMAP:
		break;
	case XDK_EVENT_MAP:
		break;
	case XDK_EVENT_MAP_REQUEST:
		break;
	case XDK_EVENT_REPARENT:
		break;
	case XDK_EVENT_CONFIGURE:
		break;
	case XDK_EVENT_CONFIGURE_REQUEST:
		break;
	case XDK_EVENT_GRAVITY:
		break;
	case XDK_EVENT_RESIZE_REQUEST:
		break;
	case XDK_EVENT_CIRCULATE:
		break;
	case XDK_EVENT_CIRCULATE_REQUEST:
		break;
	case XDK_EVENT_PROPERTY:
		break;
	case XDK_EVENT_SELECTION_CLEAR:
		break;
	case XDK_EVENT_SELECTION_REQUEST:
		break;
	case XDK_EVENT_SELECTION:
		break;
	case XDK_EVENT_COLORMAP:
		break;
	case XDK_EVENT_CLIENT_MESSAGE:
		if(event->xclient.data.l[0] ==
				xdk_display_atom_get(self->priv->display, XDK_ATOM_WM_DELETE_WINDOW)) {
			g_signal_emit(self, signals[XDK_WINDOW_DELETE_EVENT], 0, event);
		}
		break;
	case XDK_EVENT_MAPPING:
		break;
	case XDK_EVENT_GENERIC:
		break;
	default:
		g_return_if_reached();
	}
}

void xdk_window_set_gravity(XdkWindow * self, XdkGravity gravity)
{
}

XdkGravity xdk_window_get_gravity(XdkWindow self)
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
	XSetWindowAttributes * attributes)
{
}

void xdk_window_set_parent(XdkWindow * self, XdkWindow * parent)
{
	g_return_if_fail(self);
	
	self->priv->parent = parent;
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
	
	self->priv->background_color = background_color;
	if(! xdk_window_is_realized(self)) {
		return;
	}
	
	XSetWindowAttributes attributes = {
		.background_pixel = background_color,
	};
	xdk_window_set_attributes(
		self,
		XDK_ATTR_MASK_BACKGROUND_COLOR,
		& attributes);
}

gulong xdk_window_get_background_color(XdkWindow * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->background_color;
}

void xdk_window_event_set_mask(XdkWindow * self, XdkEventMask event_mask)
{
	g_return_if_fail(self);
	
	self->priv->event_mask = event_mask;
}

void xdk_window_event_add_mask(XdkWindow * self, XdkEventMask event_mask)
{
	g_return_if_fail(self);
	
	self->priv->event_mask |= event_mask;
}

void xdk_window_event_remove_mask(XdkWindow * self, XdkEventMask event_mask)
{
	g_return_if_fail(self);
	
	self->priv->event_mask &= ~event_mask;
}

gulong xdk_window_event_get_mask(XdkWindow * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->priv->event_mask;
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

	self->priv->screen = screen;
}

void xdk_window_set_wm_protocols(XdkWindow * self, Atom * protocols, gint n_protocols)
{
	g_return_if_fail(self);
	g_return_if_fail(protocols);
	
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
