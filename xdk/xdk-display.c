#include "xdk.h"
#include "xdk-base-private.h"
#include "xdk-display-private.h"

static XdkDisplay * default_display = NULL;

gboolean xdk_display_init_once()
{
	if(! default_display) {
		default_display = xdk_base_new(XDK_TYPE_DISPLAY);
	}
	
	return NULL != default_display;
}

gboolean xdk_display_init(gpointer base)
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

void xdk_display_destroy(gpointer base)
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

const XdkTypeInfo xdk_type_display = {
	XDK_TYPE_BASE,
	"XdkDisplay",
	xdk_display_init,
	xdk_display_destroy,
	sizeof(XdkDisplay)
};
