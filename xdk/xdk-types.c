#include "xdk-types.h"

static XEvent * x_event_copy(XEvent * event)
{
	g_return_val_if_fail(event, NULL);
	
	return g_slice_dup(XEvent, event);
}

static void x_event_free(XEvent * event)
{
	g_return_if_fail(event);
	
	g_slice_free(XEvent, event);
}

GType x_event_get_type()
{
	static volatile gsize type_id = 0;
	if(g_once_init_enter(& type_id)) {
		GType tmp = g_boxed_type_register_static(
			"XEvent",
			(GBoxedCopyFunc) x_event_copy,
			(GBoxedFreeFunc) x_event_free);
		g_once_init_leave(& type_id, tmp);
	}
	
	return type_id;
}
