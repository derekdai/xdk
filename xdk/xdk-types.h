#ifndef __XDK_TYPES_H_
#define __XDK_TYPES_H_

#include <glib-object.h>
#include <X11/Xlib.h>

G_BEGIN_DECLS

#define X_TYPE_EVENT		(x_event_get_type())

typedef enum _XdkGravity XdkGravity;

typedef enum _XdkWindowClasses XdkWindowClasses;

typedef enum _XdkWindowAttributeMask XdkWindowAttributeMask;

typedef enum _XdkEventMask XdkEventMask;

typedef enum _XdkEventType XdkEventType;

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

enum _XdkEventType
{
	XDK_EVENT_KEY_PRESS			= KeyPress,
	XDK_EVENT_KEY_RELEASE		= KeyRelease,
	XDK_EVENT_BUTTON_PRESS		= ButtonPress,
	XDK_EVENT_BUTTON_RELEASE	= ButtonRelease,
	XDK_EVENT_MOTION			= MotionNotify,
	XDK_EVENT_ENTER				= EnterNotify,
	XDK_EVENT_LEAVE				= LeaveNotify,
	XDK_EVENT_FOCUS_IN			= FocusIn,
	XDK_EVENT_FOCUS_OUT			= FocusOut,
	XDK_EVENT_KEYMAP			= KeymapNotify,
	XDK_EVENT_EXPOSE			= Expose,
	XDK_EVENT_GRAPHICS_EXPOSE	= GraphicsExpose,
	XDK_EVENT_NO_EXPOSE			= NoExpose,
	XDK_EVENT_VISIBILITY		= VisibilityNotify,
	XDK_EVENT_CREATE			= CreateNotify,
	XDK_EVENT_DESTROY			= DestroyNotify,
	XDK_EVENT_UNMAP				= UnmapNotify,
	XDK_EVENT_MAP				= MapNotify,
	XDK_EVENT_MAP_REQUEST		= MapRequest,
	XDK_EVENT_REPARENT			= ReparentNotify,
	XDK_EVENT_CONFIGURE			= ConfigureNotify,
	XDK_EVENT_CONFIGURE_REQUEST	= ConfigureRequest,
	XDK_EVENT_GRAVITY			= GravityNotify,
	XDK_EVENT_RESIZE_REQUEST	= ResizeRequest,
	XDK_EVENT_CIRCULATE			= CirculateNotify,
	XDK_EVENT_CIRCULATE_REQUEST = CirculateRequest,
	XDK_EVENT_PROPERTY			= PropertyNotify,
	XDK_EVENT_SELECTION_CLEAR	= SelectionClear,
	XDK_EVENT_SELECTION_REQUEST	= SelectionRequest,
	XDK_EVENT_SELECTION			= SelectionNotify,
	XDK_EVENT_COLORMAP			= ColormapNotify,
	XDK_EVENT_CLIENT_MESSAGE	= ClientMessage,
	XDK_EVENT_MAPPING			= MappingNotify,
	XDK_EVENT_GENERIC			= GenericEvent,
	XDK_EVENT_LAST				= LASTEvent
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

enum _XdkWindowClasses
{
	XDK_WINDOW_CLASSES_INPUT			= InputOnly,
	
	XDK_WINDOW_CLASSES_INPUT_OUTPUT		= InputOutput
};

GType x_event_get_type();

G_END_DECLS

#endif /* __XDK_TYPES_H_ */
