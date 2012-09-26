#include <xdk/xdk.h>
#include <clutter/clutter.h>

typedef struct _ArtCompositeManagerClass ArtCompositeManagerClass;

typedef struct _ArtCompositeManager ArtCompositeManager;

typedef struct _ArtCompositeManagerPrivate ArtCompositeManagerPrivate;

struct _ArtCompositeManagerClass
{
	GObjectClass base;
};

struct _ArtCompositeManager
{
	GObject base;
	
	ArtCompositeManagerPrivate * priv;
};

GType art_composite_manager_get_type();

G_DEFINE_TYPE(ArtCompositeManager, art_composite_manager, G_TYPE_OBJECT);

static void art_composite_manager_class_init(ArtCompositeManagerClass * clazz)
{
	
}

static void art_composite_manager_init(ArtCompositeManager * self)
{
}

int main(gint argc, gchar * args[])
{
	clutter_x11_set_use_argb_visual();
	if(CLUTTER_INIT_SUCCESS != clutter_init(& argc, & args)) {
		g_error("Failed to initlize clutter");
	}
	
	xdk_disable_event_retrieval();
	xdk_set_xdisplay(clutter_x11_get_default_display());
	xdk_init(& argc, & args);
	
	
	clutter_main();
}
