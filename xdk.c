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
	
	Display * x_display;
	
	gint n_screens;
	
	gint default_screen;
	
	XdkScreen ** screens;
};

struct _XdkScreen
{
	XdkBase parent;
	
	XdkDisplay display;
	
	gint screen_number;
	
	Screen * x_screen;
};

static XdkInitFunc xdk_type_get_init_func(XdkType type);
static XdkDestroyFunc xdk_type_get_destroy_func(XdkType type);
static XdkType xdk_type_get_parent(XdkType type);

static gboolean xdk_display_init(gpointer base);
static void xdk_display_destroy(gpointer base);

static void xdk_screen_set_x_screen(XdkScreen * screen, Screen * x_screen);

static const XdkTypeInfo type_infos[XDK_TYPE_MAX] = {
	{ XDK_TYPE_INVALID, "XdkInvalid", NULL, NULL, 0 },
	{ XDK_TYPE_INVALID, "XdkBase", NULL, NULL, 0 },
	{ XDK_TYPE_BASE, "XdkDisplay", xdk_display_init, xdk_display_destroy, sizeof(XdkDisplay) },
	{ XDK_TYPE_BASE, "XdkScreen", NULL, NULL, sizeof(XdkScreen) },
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
	self->x_display = XOpenDisplay(NULL);
	if(! self->x_display) {
		goto end;
	}
	
	self->name = XDisplayString(self->x_display);
	self->default_screen = XDefaultScreen(self->x_display);
	self->n_screens = XScreenCount(self->x_display);
	self->screens = g_malloc0(sizeof(XdkScreen *) * self->n_screens);
	
	int i;
	for(i = 0; i < self->n_screens; i ++) {
		XdkScreen * screen = xdk_base_new(XDK_TYPE_SCREEN);
		if(! screen) {
			goto end;
		}
		xdk_screen_set_x_screen(
			screen,
			XScreenOfDisplay(self->x_display, i));
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
	
	if(self->x_display) {
		XCloseDisplay(self->x_display);
	}
}

XdkDisplay * xdk_display_get_default()
{
	if(! default_display) {
		g_warning("Xdk not initialized yet");
	}
	
	return default_display;
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

static void xdk_screen_set_x_screen(XdkScreen * self, Screen * x_screen)
{
	g_return_if_fail(self);
	g_return_if_fail(x_screen);
	
	self->x_screen = x_screen;
}

gint xdk_screen_get_number(XdkScreen * self)
{
	g_return_val_if_fail(self, -1);
	g_return_val_if_fail(self->x_screen, -1);
	
	return XScreenNumberOfScreen(self->x_screen);
}

gint xdk_screen_get_width(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->x_screen, 0);
	
	return XWidthOfScreen(self->x_screen);
}

gint xdk_screen_get_height(XdkScreen * self)
{
	g_return_val_if_fail(self, 0);
	g_return_val_if_fail(self->x_screen, 0);
	
	return HeightOfScreen(self->x_screen);
}
