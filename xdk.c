#include <stdlib.h>
#include "xdk.h"

typedef struct _XdkTypeInfo XdkTypeInfo;

struct _XdkTypeInfo
{
	XdkType parent;
	const char * name;
	XdkInitFunc init_func;
	XdkDestroyFunc destroy_func;
	gsize size;
};

struct _XdkBase
{
	XdkType type;
	int refcount;
};

struct _XdkDisplay
{
	XdkBase parent;
	
	gchar * name;
	
	Display * peer;
	
	gint n_screens;
	
	gint default_screen;
	
	XdkScreen ** screens;
};

struct _XdkScreen
{
	XdkBase parent;
	
	XdkDisplay * display;
	
	Screen * peer;
	
	XdkVisual * default_visual;
	
	XdkGc * default_gc;
	
	XdkWindow * root;
};

struct _XdkWindow
{
	XdkBase parent;
	
	XdkDisplay * display;
	
	XdkScreen * screen;
	
	XdkWindow * parent_window;
	
	Window peer;
	
	gint x;
	
	gint y;
	
	guint width;
	
	guint height;
	
	gint depth;
	
	XdkVisual * visual;
	
	gulong background_color;
	
	gboolean mapped : 1;
	
	gboolean visible : 1;
	
	gboolean own_peer : 1;
};

struct _XdkGc
{
	XdkBase parent;
};

struct _XdkVisual
{
	XdkBase parent;
};

static XdkInitFunc xdk_type_get_init_func(XdkType type);
static XdkDestroyFunc xdk_type_get_destroy_func(XdkType type);
static XdkType xdk_type_get_parent(XdkType type);

static gboolean xdk_display_init(gpointer base);
static void xdk_display_destroy(gpointer base);

static void xdk_screen_set_peer(XdkScreen * screen, Screen * peer);

static gboolean xdk_window_init(gpointer base);

static const XdkTypeInfo type_infos[XDK_TYPE_MAX] = {
	{ XDK_TYPE_INVALID, "XdkInvalid", NULL, NULL, 0 },
	{ XDK_TYPE_INVALID, "XdkBase", NULL, NULL, 0 },
	{ XDK_TYPE_BASE, "XdkDisplay", xdk_display_init, xdk_display_destroy, sizeof(XdkDisplay) },
	{ XDK_TYPE_BASE, "XdkScreen", NULL, NULL, sizeof(XdkScreen) },
	{ XDK_TYPE_BASE, "XdkWindow", xdk_window_init, NULL, sizeof(XdkWindow) },
	{ XDK_TYPE_BASE, "XdkGc", NULL, NULL, sizeof(XdkGc) },
	{ XDK_TYPE_BASE, "XdkVisual", NULL, NULL, sizeof(XdkVisual) },
};

static XdkDisplay * default_display = NULL;

static const char * xdk_type_get_name(XdkType type)
{
	g_return_val_if_fail(type >= XDK_TYPE_INVALID && type < XDK_TYPE_MAX, NULL);
	
	return type_infos[type].name;
}

static XdkInitFunc xdk_type_get_init_func(XdkType type)
{
	g_return_val_if_fail(type >= XDK_TYPE_INVALID && type < XDK_TYPE_MAX, NULL);
	
	return type_infos[type].init_func;
}

static XdkDestroyFunc xdk_type_get_destroy_func(XdkType type)
{
	g_return_val_if_fail(type >= XDK_TYPE_INVALID && type < XDK_TYPE_MAX, NULL);
	
	return type_infos[type].destroy_func;
}

static XdkType xdk_type_get_parent(XdkType type)
{
	g_return_val_if_fail(type >= XDK_TYPE_INVALID && type < XDK_TYPE_MAX, XDK_TYPE_INVALID);
	
	return type_infos[type].parent;
}

static gboolean _xdk_base_init(gpointer base, XdkType type)
{
	if(XDK_TYPE_INVALID == type) {
		return TRUE;
	}
	else {
		_xdk_base_init(base, xdk_type_get_parent(type));
	}
	
	XdkInitFunc init_func = xdk_type_get_init_func(type);
	
	return init_func ? init_func(base) : TRUE;
}

static void _xdk_base_free(gpointer base, XdkType type)
{
	g_return_if_fail(base);
	
	XdkType curr_type = type;
	while(curr_type != XDK_TYPE_INVALID) {
		XdkDestroyFunc destroy_func = xdk_type_get_destroy_func(curr_type);
		if(destroy_func) {
			destroy_func(base);
		}
		curr_type = xdk_type_get_parent(curr_type);
	}
	
	g_free(base);
}

gpointer xdk_base_new(XdkType type)
{
	g_return_val_if_fail(type > XDK_TYPE_BASE && type < XDK_TYPE_MAX, NULL);
	
	gpointer base = g_malloc0(type_infos[type].size);
	if(base) {
		XDK_BASE(base)->type = type;
		XDK_BASE(base)->refcount = 1;
	}
	
	if(! _xdk_base_init(base, type)) {
		_xdk_base_free(base, type);
		base = NULL;
	}
	
	return base;
}

gpointer xdk_base_ref(gpointer base)
{
	g_return_val_if_fail(base, NULL);
	
	XdkBase * self = XDK_BASE(base);
	
	g_return_val_if_fail(self->type > XDK_TYPE_INVALID && self->type < XDK_TYPE_MAX, NULL);
	g_return_val_if_fail(self->refcount > 0, NULL);
	
	self->refcount ++;
	
	return base;
}

void xdk_base_unref(gpointer base)
{
	g_return_if_fail(base);
	
	XdkBase * self = XDK_BASE(base);
	
	g_return_if_fail(self->type > XDK_TYPE_INVALID && self->type < XDK_TYPE_MAX);
	g_return_if_fail(self->refcount > 0);
	
	self->refcount --;
	if(! self->refcount) {
		_xdk_base_free(base, self->type);
	}
}

gboolean xdk_init(int * argc, char ** args[])
{
	if(! default_display) {
		default_display = xdk_base_new(XDK_TYPE_DISPLAY);
	}

	return NULL != default_display;
}

static gboolean xdk_display_init(gpointer base)
{
	g_return_val_if_fail(base, FALSE);

	gboolean result = FALSE;
	XdkDisplay * self = XDK_DISPLAY(base);
	self->peer = XOpenDisplay(NULL);
	if(! self->peer) {
		goto end;
	}
	
	self->name = XDisplayString(self->peer);
	self->default_screen = XDefaultScreen(self->peer);
	self->n_screens = XScreenCount(self->peer);
	self->screens = g_malloc0(sizeof(XdkScreen *) * self->n_screens);
	
	int i;
	for(i = 0; i < self->n_screens; i ++) {
		XdkScreen * screen = xdk_base_new(XDK_TYPE_SCREEN);
		if(! screen) {
			goto end;
		}
		xdk_screen_set_peer(
			screen,
			XScreenOfDisplay(self->peer, i));
		self->screens[i] = screen;
	}
	
	result = TRUE;
	
end:
	return result;
}

static void xdk_display_destroy(gpointer base)
{
	g_return_if_fail(base);
	
	XdkDisplay * self = XDK_DISPLAY(base);
	
	int i;
	for(i = 0; i < self->n_screens; i ++) {
		if(self->screens[i]) {
			xdk_base_unref(self->screens[i]);
		}
	}
	g_free(self->screens);
	
	if(self->peer) {
		XCloseDisplay(self->peer);
	}
}

XdkDisplay * xdk_display_get_default()
{
	if(! default_display) {
		g_warning("Xdk not initialized yet");
	}
	
	return default_display;
}

Display * xdk_display_get_peer(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->peer;
}

const gchar * xdk_display_get_name(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->name;
}

gint xdk_display_get_n_screens(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->n_screens;
}

XdkScreen * xdk_display_get_default_screen(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return self->screens[self->default_screen];
}

void xdk_display_flush(XdkDisplay * self)
{
	g_return_if_fail(self);
	
	XFlush(self->peer);
}

const char * xdk_display_get_vendor(XdkDisplay * self)
{
	g_return_val_if_fail(self, NULL);
	
	return XServerVendor(self->peer);
}

gint xdk_display_get_release(XdkDisplay * self)
{
	g_return_val_if_fail(self, 0);
	
	return XVendorRelease(self->peer);
}

XdkWindow * xdk_get_default_root_window()
{
	XdkDisplay * display = xdk_display_get_default();
	XdkScreen * screen = xdk_display_get_default_screen(display);
	
	return xdk_screen_get_root_window(screen);
}

static void xdk_screen_set_peer(XdkScreen * self, Screen * peer)
{
	g_return_if_fail(self);
	g_return_if_fail(peer);
	
	self->peer = peer;
}

Screen * xdk_screen_get_peer(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->peer;
}

gint xdk_screen_get_number(XdkScreen * self)
{
	g_return_val_if_fail(self, -1);
	g_return_val_if_fail(self->peer, -1);
	
	return XScreenNumberOfScreen(self->peer);
}

gint xdk_screen_get_width(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->peer, 0);
	
	return XWidthOfScreen(self->peer);
}

gint xdk_screen_get_height(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->peer, 0);
	
	return HeightOfScreen(self->peer);
}

gint xdk_screen_get_default_depth(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->peer, 0);
	
	return XDefaultDepthOfScreen(self->peer);
}

XdkGc * xdk_screen_get_default_gc(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->default_gc;
}

XdkVisual * xdk_screen_get_default_visual(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->default_visual;
}

XdkDisplay * xdk_screen_get_display(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->display;
}

glong xdk_screen_get_event_mask(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	
	return XEventMaskOfScreen(self->peer);
}

XdkWindow * xdk_screen_get_root_window(XdkScreen * self)
{
	g_return_val_if_fail(self, NULL);
	
	if(! self->root) {
		self->root = xdk_base_new(XDK_TYPE_WINDOW);
		xdk_window_set_peer(self->root, XRootWindowOfScreen(self->peer));
	}
	
	return self->root;
}

static gboolean xdk_window_init(gpointer base)
{
	g_return_val_if_fail(base, FALSE);
	
	XdkWindow * self = XDK_WINDOW(base);
	self->display = xdk_display_get_default();
	self->screen = xdk_display_get_default_screen(self->display);
	self->peer = None;
	self->background_color = 0xffffffff;
	
	return TRUE;
}

XdkWindow * xdk_window_new()
{
	return xdk_base_new(XDK_TYPE_WINDOW);
}

void xdk_window_set_peer(XdkWindow * self, Window peer)
{
	g_return_if_fail(self);
	g_return_if_fail(! self->peer);
	
	self->peer = peer;
	self->own_peer = FALSE;
}

Window xdk_window_get_peer(XdkWindow * self)
{
	g_return_val_if_fail(self, None);
	
	return self->peer;
}

gboolean xdk_window_is_realized(XdkWindow * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return None != self->peer;
}

/**
 * http://tronche.com/gui/x/xlib/window/XCreateWindow.html
 */
void xdk_window_realize_simple(XdkWindow * self)
{
	g_return_if_fail(self);
	
	XdkWindow * parent = self->parent_window;
	if(! parent) {
		// 
		parent = xdk_get_default_root_window();
	}
	
	self->peer = XCreateSimpleWindow(
		xdk_display_get_peer(self->display),
		xdk_window_get_peer(parent),
		self->x, self->y,
		self->width, self->height,
		0, 0,
		self->background_color);
	self->own_peer = TRUE;
}

void xdk_window_get_position(XdkWindow * self, int * x, int * y)
{
	g_return_if_fail(self);
	
	if(x) {
		* x = self->x;
	}
	
	if(y) {
		* y = self->y;
	}
}

void xdk_window_set_position(XdkWindow * self, int x, int y)
{
	g_return_if_fail(self);
	
	self->x = x;
	self->y = y;
}

void xdk_window_get_size(XdkWindow * self, int * width, int * height)
{
	g_return_if_fail(self);

	if(width) {
		* width = self->width;
	}
	
	if(height) {
		* height = self->height;
	}
}

void xdk_window_set_size(XdkWindow * self, int width, int height)
{
	g_return_if_fail(self);
	
	self->width = width;
	self->height = height;
}

void xdk_window_map(XdkWindow * self)
{
	g_return_if_fail(self);
	
	if(! self->mapped) {
		XMapWindow(xdk_display_get_peer(self->display), self->peer);
	}
}

void xdk_window_show(XdkWindow * self)
{
	g_return_if_fail(self);
}

void xdk_window_hide(XdkWindow * self)
{
}
