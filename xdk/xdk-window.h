#ifndef __XDK_WINDOW_H_
#define __XDK_WINDOW_H_

#include <glib.h>
#include <X11/Xlib.h>

G_BEGIN_DECLS

#define XDK_TYPE_WINDOW (xdk_window_get_type())
#define XDK_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), XDK_TYPE_WINDOW, XdkWindow))
#define XDK_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XDK_TYPE_WINDOW, XdkWindowClass))
#define IS_XDK_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XDK_TYPE_WINDOW))
#define IS_XDK_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XDK_TYPE_WINDOW))
#define XDK_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), XDK_TYPE_WINDOW, XdkWindowClass))

typedef struct _XdkWindowClass XdkWindowClass;

typedef struct _XdkWindow XdkWindow;

typedef struct _XdkWindowPrivate XdkWindowPrivate;

typedef enum _XdkGravity XdkGravity;

typedef enum _XdkWindowType XdkWindowType;

typedef enum _XdkWindowAttributeMask XdkWindowAttributeMask;

enum _XdkGravity
{
	XDK_GRAVITY_FORGET		= ForgetGravity,
	XDK_GRAVITY_NORTH_WEST	= NorthWestGravity,
	XDK_GRAVITY_NORTH		= NorthGravity,
	XDK_GRAVITY_NORTH_EAST	= NorthEastGravity,
	XDK_GRAVITY_WEST		= WestGravity,
	XDK_GRAVITY_CENTER		= CenterGravity,
	XDK_GRAVITY_EAST		= EastGravity,
	XDK_GRAVITY_SOUTH_WEST	= SouthWestGravity,
	XDK_GRAVITY_SOUTH		= SouthGravity,
	XDK_GRAVITY_SOUTH_EAST	= SouthEastGravity,
};

/**
 * http://tronche.com/gui/x/xlib/events/mask.html
 */
enum _XdkEventMask
{
	XDK_EVENT_MASK_NO						= NoEventMask,		//		No events wanted
	XDK_EVENT_MASK_KEY_PRESS				= KeyPressMask,		//		Keyboard down events wanted
	XDK_EVENT_MASK_KEY_RELEASE				= KeyReleaseMask,	//		Keyboard up events wanted
	XDK_EVENT_MASK_BUTTON_PRESS				= ButtonPressMask,	//		Pointer button down events wanted
	XDK_EVENT_MASK_BUTTON_RELEASE			= ButtonReleaseMask,//		Pointer button up events wanted
	XDK_EVENT_MASK_ENTER_WINDOW				= EnterWindowMask,	//		Pointer window entry events wanted
	XDK_EVENT_MASK_LEAVE_WINDOW				= LeaveWindowMask,	//		Pointer window leave events wanted
	XDK_EVENT_MASK_MOTION					= PointerMotionMask,//		Pointer motion events wanted
	XDK_EVENT_MASK_MOTION_HINT				= PointerMotionHintMask,//		Pointer motion hints wanted
	XDK_EVENT_MASK_BUTTON1_MOTION			= Button1MotionMask,//		Pointer motion while button 1 down
	XDK_EVENT_MASK_BUTTON2_MOTION			= Button2MotionMask,//		Pointer motion while button 2 down
	XDK_EVENT_MASK_BUTTON3_MOTION			= Button3MotionMask,//		Pointer motion while button 3 down
	XDK_EVENT_MASK_BUTTON4_MOTION			= Button4MotionMask,//		Pointer motion while button 4 down
	XDK_EVENT_MASK_BUTTON5_MOTION			= Button5MotionMask,//		Pointer motion while button 5 down
	XDK_EVENT_MASK_BUTTON_MOTION			= ButtonMotionMask, //		Pointer motion while any button down
	XDK_EVENT_MASK_KEYMAP_STATE				= KeymapStateMask,	//		Keyboard state wanted at window entry and focus in
	XDK_EVENT_MASK_EXPOSURE					= ExposureMask,		//		Any exposure wanted
	XDK_EVENT_MASK_VISIBILITY_CHANGE		= VisibilityChangeMask,	//		Any change in visibility wanted
	XDK_EVENT_MASK_STRUCTURE_NOTIFY			= StructureNotifyMask,	//		Any change in window structure wanted
	XDK_EVENT_MASK_RESIZE_REDIRECT			= ResizeRedirectMask,	//		Redirect resize of this window
	XDK_EVENT_MASK_SUBSTRUCTURE_NOTIFY		= SubstructureNotifyMask,	//		Substructure notification wanted
	XDK_EVENT_MASK_SUBSTRUCTURE_REDIRECT	= SubstructureRedirectMask,	//		Redirect structure requests on children
	XDK_EVENT_MASK_FOCUS_CHANGE				= FocusChangeMask,		//		Any change in input focus wanted
	XDK_EVENT_MASK_PROPERTY_CHANGE			= PropertyChangeMask,	//		Any change in property wanted
	XDK_EVENT_MASK_COLORMAP_CHANGE			= ColormapChangeMask,	//		Any change in colormap wanted
	XDK_EVENT_MASK_OWNER_GRUB_BUTTON		= OwnerGrabButtonMask,
};

/**
 * http://tronche.com/gui/x/xlib/window/attributes/
 * 
 * Attribute				Default 			InputOutput		InputOnly
 * background-pixmap		None				Yes				No
 * background-pixel			Undefined 			Yes 			No
 * border-pixmap			CopyFromParent		Yes 			No
 * border-pixel				Undefined 			Yes 			No
 * bit-gravity				ForgetGravity		Yes 			No
 * win-gravity				NorthWestGravity	Yes 			Yes
 * backing-store			NotUseful			Yes 			No
 * backing-planes			All ones 			Yes 			No
 * backing-pixel			zero 				Yes 			No
 * save-under				False				Yes 			No
 * event-mask				empty set 			Yes 			Yes
 * do-not-propagate-mask	empty set 			Yes 			Yes
 * override-redirect		False				Yes 			Yes
 * colormap					CopyFromParent		Yes 			No
 * cursor					None				Yes 			Yes 
 **/
enum _XdkWindowAttributeMask
{
	XDK_ATTR_MASK_BACKGROUND_IMAGE		= CWBackPixmap,
	XDK_ATTR_MASK_BACKGROUND_COLOR		= CWBackPixel,
	XDK_ATTR_MASK_BORDER_IMAGE			= CWBorderPixmap,
	XDK_ATTR_MASK_BORDER_PIXEL			= CWBorderPixel,
	XDK_ATTR_MASK_BIT_GRAVITY			= CWBitGravity,
	XDK_ATTR_MASK_WIN_GRAVITY			= CWWinGravity,
	XDK_ATTR_MASK_BACKING_STORE			= CWBackingStore,
	XDK_ATTR_MASK_BACKING_PLANES		= CWBackingPlanes,
	XDK_ATTR_MASK_BACKING_COLOR			= CWBackingPixel,
	XDK_ATTR_MASK_OVERRIDE_REDIRECT		= CWOverrideRedirect,
	XDK_ATTR_MASK_SAVE_UNDER			= CWSaveUnder,
	XDK_ATTR_MASK_EVENT_MASK			= CWEventMask,
	XDK_ATTR_MASK_DONT_PROPAGATE_MASK 	= CWDontPropagate,
	XDK_ATTR_MASK_COLORMAP				= CWColormap,
	XDK_ATTR_MASK_CURSOR				= CWCursor,
};

enum _XdkWindowType
{
	XDK_WINDOW_TYPE_INPUT			= InputOnly,
	
	XDK_WINDOW_TYPE_INPUT_OUTPUT	= InputOutput
};

struct _XdkWindowClass
{
	GObjectClass base;
	
	void (* realize)(XdkWindow * self);
};

struct _XdkWindow
{
	GObject base;
	
	XdkWindowPrivate * priv;
};

GType xdk_window_get_type();

XdkWindow * xdk_window_new();

void xdk_window_set_foreign_peer(XdkWindow * self, Window peer);

void xdk_window_take_peer(XdkWindow * self, Window peer);

Window xdk_window_get_peer(XdkWindow * self);

void xdk_window_realize(XdkWindow * self);

gboolean xdk_window_is_realized(XdkWindow * self);

void xdk_window_unrealize(XdkWindow * self);

void xdk_window_map(XdkWindow * self);

gboolean xdk_window_is_mapped(XdkWindow * self);

void xdk_window_unmap(XdkWindow * self);

void xdk_window_destroy(XdkWindow * self);

void xdk_window_get_position(XdkWindow * self, int * x, int * y);

void xdk_window_set_position(XdkWindow * self, int x, int y);

void xdk_window_get_size(XdkWindow * self, int * width, int * height);

void xdk_window_set_size(XdkWindow * self, int width, int height);

void xdk_window_handle_event(XdkWindow * self, XEvent * event);

void xdk_window_set_attributes(
	XdkWindow * self,
	XdkWindowAttributeMask mask,
	const XSetWindowAttributes * attributes);

void xdk_window_get_attributes(
	XdkWindow * self,
	XSetWindowAttributes * attributes);

void xdk_window_set_gravity(XdkWindow * self, XdkGravity gravity);

XdkGravity xdk_window_get_gravity(XdkWindow self);

Atom * xdk_window_list_properties(XdkWindow * self, int * n_props);

Window xdk_window_create_window(
	XdkWindow * self,
	XdkWindow * parent,
	XdkWindowType window_type,
	XdkWindowAttributeMask attribute_mask,
	XSetWindowAttributes *attributes);

void xdk_window_set_parent(XdkWindow * self, XdkWindow * parent);

XdkWindow * xdk_window_get_parent(XdkWindow * self);
G_END_DECLS

#endif /* __XDK_WINDOW_H_ */
