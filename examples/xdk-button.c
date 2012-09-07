#include <xdk/xdk.h>

#define MY_TYPE_BUTTON (my_button_get_type())
#define MY_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MY_TYPE_BUTTON, MyButton))
#define MY_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MY_TYPE_BUTTON, MyButtonClass))
#define IS_MY_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MY_TYPE_BUTTON))
#define IS_MY_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MY_TYPE_BUTTON))
#define MY_BUTTON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MY_TYPE_BUTTON, MyButtonClass))

typedef struct _MyButtonClass MyButtonClass;

typedef struct _MyButton MyButton;

struct _MyButtonClass
{
	XdkWindowClass base;
};

struct _MyButton
{
	XdkWindow base;
	
	gboolean pressed;
	
	gboolean entered;
};

enum MyButtonSignal
{
	MY_BUTTON_SIGNAL_CLICKED,
	MY_BUTTON_SIGNAL_LAST
};

static void my_button_realize(XdkWindow * window);

static void my_button_press(XdkWindow * window, XEvent * event);

static void my_button_release(XdkWindow * window, XEvent * event);

static void my_button_enter(XdkWindow * window, XEvent * event);

static void my_button_leave(XdkWindow * window, XEvent * event);

G_DEFINE_TYPE(MyButton, my_button, XDK_TYPE_WINDOW);

static guint my_button_signals[MY_BUTTON_SIGNAL_LAST];

void my_button_class_init(MyButtonClass * clazz)
{
	XdkWindowClass * xdk_window_class = XDK_WINDOW_CLASS(clazz);
	//xdk_window_class->realize = my_button_realize;
	xdk_window_class->button_press = my_button_press;
	xdk_window_class->button_release = my_button_release;
	xdk_window_class->enter = my_button_enter;
	xdk_window_class->leave = my_button_leave;
	
	my_button_signals[MY_BUTTON_SIGNAL_CLICKED] = g_signal_new(
		"clicked",
		G_TYPE_FROM_CLASS(clazz),
		G_SIGNAL_RUN_LAST,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1, X_TYPE_EVENT);
}

static void my_button_init(MyButton * self)
{
	self->pressed = FALSE;
	self->entered = FALSE;
	xdk_window_event_mask_set(
		XDK_WINDOW(self),
		XDK_EVENT_MASK_STRUCTURE_NOTIFY |
		XDK_EVENT_MASK_BUTTON_PRESS | XDK_EVENT_MASK_BUTTON_RELEASE |
		XDK_EVENT_MASK_ENTER_WINDOW | XDK_EVENT_MASK_LEAVE_WINDOW);
}

static void my_button_realize(XdkWindow * window)
{
}

static void my_button_press(XdkWindow * window, XEvent * event)
{
	MY_BUTTON(window)->pressed = TRUE;
}

static void my_button_release(XdkWindow * window, XEvent * event)
{
	MyButton * self = MY_BUTTON(window);
	if(! self->pressed) {
		return;
	}
	
	self->pressed = FALSE;
	
	if(! self->entered) {
		return;
	}
	
	g_signal_emit(self, my_button_signals[MY_BUTTON_SIGNAL_CLICKED], 0, event);
}

static void my_button_enter(XdkWindow * window, XEvent * event)
{
	MY_BUTTON(window)->entered = TRUE;
}

static void my_button_leave(XdkWindow * window, XEvent * event)
{
	MY_BUTTON(window)->entered = FALSE;
}

XdkWindow * my_button_new()
{
	return g_object_new(MY_TYPE_BUTTON, NULL);
}

void on_quit(XdkWindow * window)
{
	g_message("Bye");
	
	xdk_main_quit();
}

int main(int argc, char * args[])
{
	xdk_init(& argc, & args);
	
	XdkWindow * button = my_button_new();
	xdk_window_set_size(button, 320, 240);
	g_signal_connect(button, "delete-event", G_CALLBACK(xdk_window_destroy), NULL);
	g_signal_connect(button, "destroy", G_CALLBACK(on_quit), NULL);
	g_signal_connect(button, "clicked", G_CALLBACK(on_quit), NULL);
	xdk_window_show(button);
	
	xdk_main();
	
	g_object_unref(button);
	
	return 0;
}
