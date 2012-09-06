#include "xdk-types.h"
#include "xdk-display.h"

static const char * xdk_error_names[] = {
	"XDK_ERROR_SUCCESS (Success)",
	"XDK_ERROR_REQUEST (BadRequest)",
	"XDK_ERROR_VALUE (BadValue)",
	"XDK_ERROR_WINDOW (BadWindow)",
	"XDK_ERROR_PIXMAP (BadPixmap)",
	"XDK_ERROR_ATOM (BadAtom)",
	"XDK_ERROR_CUSOR (BadCursor)",
	"XDK_ERROR_FONT (BadFont)",
	"XDK_ERROR_MATCH (BadMatch)",
	"XDK_ERROR_DRAWABLE (BadDrawable)",
	"XDK_ERROR_ACCESS (BadAccess)",
	"XDK_ERROR_ALLOC (BadAlloc)",
	"XDK_ERROR_COLOR (BadColor)",
	"XDK_ERROR_GC (BadGC)",
	"XDK_ERROR_ID_CHOICE (BadIDChoice)",
	"XDK_ERROR_NAME (BadName)",
	"XDK_ERROR_LENGTH (BadLength)",
	"XDK_ERROR_IMPLEMENTAION (BadImplementation)",
};

static char xdk_error_buf[128];

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

const char * xdk_error_type_to_string(XdkErrorType type)
{
	XdkDisplay * display = xdk_display_get_default();
	XGetErrorText(
		xdk_display_get_peer(display),
		type,
		xdk_error_buf, sizeof(xdk_error_buf));
		
	return xdk_error_buf;
}

GQuark xdk_error_quark()
{
	static GQuark error_quark = 0;
	if(! error_quark) {
		error_quark = g_quark_from_static_string("XdkError");
	}
	
	return error_quark;
}

GError * xdk_error_new(XErrorEvent * error)
{
	g_return_val_if_fail(error, NULL);
	
	GError * gerror = NULL;
	if(XDK_ERROR_SUCCESS != error->error_code) {
		gerror = g_error_new(
			XDK_ERROR,
			error->error_code,
			"%s [display=%s, serial=%lu, request_code=%u, minor_code=%u, resourceid=0x%x]",
			xdk_error_type_to_string(error->error_code),
			XDisplayString(error->display),
			error->serial,
			error->request_code,
			error->minor_code,
			error->resourceid);
	}
	
	return gerror;
}
