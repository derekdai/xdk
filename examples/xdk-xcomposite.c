#include <xdk/xdk.h>
#include <X11/extensions/Xcomposite.h>

#define XDK_TYPE_MIRROR (xdk_mirror_get_type())
#define XDK_MIRROR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), XDK_TYPE_MIRROR, XdkMirror))
#define XDK_MIRROR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XDK_TYPE_MIRROR, XdkMirrorClass))
#define IS_XDK_MIRROR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XDK_TYPE_MIRROR))
#define IS_XDK_MIRROR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XDK_TYPE_MIRROR))
#define XDK_MIRROR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), XDK_TYPE_MIRROR, XdkMirrorClass))

typedef struct _XdkMirrorClass XdkMirrorClass;

typedef struct _XdkMirror XdkMirror;

struct _XdkMirrorClass
{
	XdkWindowClass base;
	
	void (* src_configured)(XdkMirror * self);
};

struct _XdkMirror
{
	XdkWindow base;
	
	XdkWindow * src_root;
	
	Pixmap content;
	
	Window overlay;
};

enum {
	PROP_SOURCE_ROOT = 1,
};

enum {
	SIGNAL_SRC_CONFIGURED,
	SIGNAL_LAST
};

GType xdk_mirror_get_type();

static void xdk_mirror_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec);

static void xdk_mirror_get_property(
	GObject * object,
	guint property_id,
	GValue * value,
	GParamSpec * pspec);

static void xdk_mirror_dispose(GObject * object);

static void xdk_mirror_finalize(GObject * object);

static void xdk_mirror_on_src_configured(XdkMirror * self);

void xdk_mirror_set_src_root(XdkMirror * self, XdkWindow * src_root);

XdkWindow * xdk_mirror_get_src_root(XdkMirror * self);

static guint signals[SIGNAL_LAST];

static gint xcomposite_event_base;

static gint xcomposite_error_base;

static gboolean xdk_mirror_event_filter(
	XdkDisplay * display,
	XEvent * event,
	gpointer user_data)
{
	xdk_util_event_dump(event);

	XdkMirror * self = XDK_MIRROR(user_data);
	if(event->xany.window != xdk_window_get_peer(self->src_root)) {
		return TRUE;
	}
	
	if(event->type != ConfigureNotify) {
		return TRUE;
	}
	
	g_signal_emit(self, signals[SIGNAL_SRC_CONFIGURED], 0);
	
	return FALSE;
}

static void xdk_mirror_class_init(XdkMirrorClass * clazz)
{
	GObjectClass * gobject_class = G_OBJECT_CLASS(clazz);
	gobject_class->dispose = xdk_mirror_dispose;
	gobject_class->finalize = xdk_mirror_finalize;
	gobject_class->set_property = xdk_mirror_set_property;
	gobject_class->get_property = xdk_mirror_get_property;
	
	clazz->src_configured = xdk_mirror_on_src_configured;
	
	g_object_class_install_property(
		gobject_class,
		PROP_SOURCE_ROOT,
		g_param_spec_object(
			"source-root", "", "",
			XDK_TYPE_WINDOW,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
			
	signals[SIGNAL_SRC_CONFIGURED] = g_signal_new(
		"src-configured",
		G_OBJECT_CLASS_TYPE(clazz),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(XdkMirrorClass, src_configured),
		NULL, NULL,
		g_cclosure_marshal_generic,
		G_TYPE_NONE, 0);
		
	XdkDisplay * display = xdk_display_get_default();	
	if(! XCompositeQueryExtension(
			xdk_display_get_peer(display),
			& xcomposite_event_base,
			& xcomposite_error_base)) {
		g_error("No XComposite suppor found");
	}
	
	int major, minor;
	XCompositeQueryVersion(xdk_display_get_peer(display), & major, & minor);
	if(major > 0 || minor > 4) {
		g_error("Only <XComponent-0.4 is supported");
	}
}

static void xdk_mirror_init(XdkMirror * self)
{
	
}

static void xdk_mirror_set_property(
	GObject * object,
	guint property_id,
	const GValue * value,
	GParamSpec * pspec)
{
	XdkMirror * self = XDK_MIRROR(object);
	
	switch(property_id) {
	case PROP_SOURCE_ROOT:
		xdk_mirror_set_src_root(self, g_value_get_object(value));
		break;
	default:
		g_return_if_reached();
	}
}

static void xdk_mirror_get_property(
	GObject * object,
	guint property_id,
	GValue * value,
	GParamSpec * pspec)
{
	XdkMirror * self = XDK_MIRROR(object);
	
	switch(property_id) {
	case PROP_SOURCE_ROOT:
		g_value_set_object(value, xdk_mirror_get_src_root(self));
		break;
	default:
		g_return_if_reached();
	}
}

static void xdk_mirror_dispose(GObject * object)
{
	XdkMirror * self = XDK_MIRROR(object);
	
	if(self->src_root) {
		g_object_unref(self->src_root);
		self->src_root = NULL;
	}
}

static void xdk_mirror_finalize(GObject * object)
{
}

static void xdk_mirror_on_src_configured(XdkMirror * self)
{
	if(! self->src_root) {
		return;
	}
	
	XdkDisplay * display = xdk_display_get_default();

	if(None != self->content) {
		XFreePixmap(xdk_display_get_peer(display), self->content);
	}
	
	self->content = XCompositeNameWindowPixmap(
		xdk_display_get_peer(display),
		xdk_window_get_peer(self->src_root));
		
	guint width, height;
	xdk_window_get_size(self->src_root, & width, & height);
	
	GC gc = XCreateGC(
		xdk_display_get_peer(display),
		xdk_window_get_peer(XDK_WINDOW(self)),
		0, NULL);
		
	XCopyArea(
		xdk_display_get_peer(display),
		self->content,
		xdk_window_get_peer(XDK_WINDOW(self)),
		gc,
		0, 0,
		width, height,
		0, 0);
		
	XFreeGC(xdk_display_get_peer(display), gc);
}

void xdk_mirror_set_src_root(XdkMirror * self, XdkWindow * src_root)
{
	g_return_if_fail(self);
	
	if(self->src_root == src_root) {
		return;
	}

	XdkDisplay * display = xdk_display_get_default();
	
	gboolean filter_installed = FALSE;
	if(self->src_root) {
		XFreePixmap(xdk_display_get_peer(display), self->content);
		self->content = None;
		
		//XCompositeReleaseOverlayWindow(
		//	xdk_display_get_peer(display),
		//	xdk_window_get_peer(self->src_root));
		
		XCompositeUnredirectWindow(
			xdk_display_get_peer(display),
			xdk_window_get_peer(self->src_root),
			CompositeRedirectAutomatic);
		
		g_object_unref(self->src_root);
		filter_installed = TRUE;
	}
	
	if(src_root) {
		g_object_ref(src_root);
	}
	
	self->src_root = src_root;
	
	if(filter_installed && ! src_root) {
		xdk_display_remove_event_filter(
			display,
			xdk_mirror_event_filter, self);
	}
	else if(! filter_installed && src_root) {
		xdk_window_realize(src_root);
		
		XCompositeRedirectWindow(
			xdk_display_get_peer(display),
			xdk_window_get_peer(src_root),
			//CompositeRedirectManual);
			CompositeRedirectAutomatic);
			
		self->content = XCompositeNameWindowPixmap(
			xdk_display_get_peer(display),
			xdk_window_get_peer(src_root));
		
		// map overlay will cover on top of all windows
		//self->overlay = XCompositeGetOverlayWindow(
		//	xdk_display_get_peer(display),
		//	xdk_window_get_peer(src_root));
		
		xdk_display_add_event_filter(
			display,
			xdk_mirror_event_filter, self);
	}
}

XdkWindow * xdk_mirror_get_src_root(XdkMirror * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->src_root;
}

G_DEFINE_TYPE(XdkMirror, xdk_mirror, XDK_TYPE_WINDOW);

static gboolean on_timeout_update_background(XdkWindow * window)
{
	glong width, height;
	xdk_window_get_size(window, & width, & height);
	if(width >= 1280 || height >= 720) {
		width = 10;
		height = 10;
	}
	xdk_window_set_size(window, width + 10, height + 10);
	xdk_flush();
	
	return TRUE;
}

int main(int argc, char * args[])
{
	xdk_init(& argc, & args);
	
	XdkWindow * src = xdk_window_new();
	xdk_window_set_size(src, 1280, 720);
	xdk_window_set_position(src, 500, 300);
	g_signal_connect(src, "delete", G_CALLBACK(xdk_window_destroy), NULL);
	g_signal_connect(src, "destroy", G_CALLBACK(xdk_main_quit), NULL);
	xdk_window_show(src);
	
	XdkWindow * dest = g_object_new(
		XDK_TYPE_MIRROR,
		"source-root", src,
		NULL);
	xdk_window_set_background_color(dest, 0x7f7f7f);
	xdk_window_set_size(dest, 640, 360);
	xdk_window_show(dest);
	
	g_timeout_add(200, (GSourceFunc) on_timeout_update_background, src);
	
	xdk_main();
	
	return 0;
}
