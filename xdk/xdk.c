#include <stdlib.h>
#include "xdk.h"
#include "xdk-base-private.h"
#include "xdk-display-private.h"
#include "xdk-screen-private.h"
#include "xdk-window-private.h"

extern const XdkTypeInfo * xdk_type_infos[XDK_TYPE_MAX];

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

static gboolean gdk_initialized = FALSE;

static const char * xdk_type_get_name(XdkType type)
{
	g_return_val_if_fail(type >= XDK_TYPE_INVALID && type < XDK_TYPE_MAX, NULL);
	
	return xdk_type_infos[type]->name;
}

static XdkInitFunc xdk_type_get_init_func(XdkType type)
{
	g_return_val_if_fail(type >= XDK_TYPE_INVALID && type < XDK_TYPE_MAX, NULL);
	
	return xdk_type_infos[type]->init_func;
}

static XdkDestroyFunc xdk_type_get_destroy_func(XdkType type)
{
	g_return_val_if_fail(type >= XDK_TYPE_INVALID && type < XDK_TYPE_MAX, NULL);
	
	return xdk_type_infos[type]->destroy_func;
}

static XdkType xdk_type_get_parent(XdkType type)
{
	g_return_val_if_fail(type >= XDK_TYPE_INVALID && type < XDK_TYPE_MAX, XDK_TYPE_INVALID);
	
	return xdk_type_infos[type]->parent;
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
	
	gpointer base = g_malloc0(xdk_type_infos[type]->size);
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
	return xdk_display_init_once();
}

const XdkTypeInfo xdk_type_invalid = {
	XDK_TYPE_INVALID,
	"XdkInvalid",
	NULL,
	NULL,
	0
};

const XdkTypeInfo xdk_type_base = {
	XDK_TYPE_INVALID,
	"XdkBase",
	NULL,
	NULL,
	0
};

const XdkTypeInfo xdk_type_gc = {
	XDK_TYPE_BASE,
	"XdkGc",
	NULL,
	NULL,
	0
};


const XdkTypeInfo xdk_type_visual = {
	XDK_TYPE_BASE,
	"XdkVisual",
	NULL,
	NULL,
	0
};
