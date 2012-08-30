#include <stdlib.h>
#include "xdk-type.h"
#include "xdk-base.h"
#include "xdk-base-private.h"
#include "xdk-display.h"

extern const XdkTypeInfo const * xdk_type_infos[XDK_TYPE_MAX];

static gboolean quited = FALSE;

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
		XdkFinalizeFunc finalize_func = xdk_type_get_finalize_func(curr_type);
		if(finalize_func) {
			finalize_func(base);
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

void xdk_main()
{
	XdkDisplay * display = xdk_display_get_default();
	
	while(! quited && xdk_display_next_event(display)) {
		xdk_display_dispatch_event(display);
	}
}

void xdk_main_quit()
{
	quited = TRUE;
}

const XdkTypeInfo xdk_type_invalid = {
	XDK_TYPE_INVALID,
	"XdkInvalid",
	0,
	NULL,
	NULL
};

const XdkTypeInfo xdk_type_base = {
	XDK_TYPE_INVALID,
	"XdkBase",
	0,
	NULL,
	NULL
};
