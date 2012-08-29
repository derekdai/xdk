#include "xdk-type.h"
#include "xdk-type-registry.h"

extern const XdkTypeInfo xdk_type_invalid;
extern const XdkTypeInfo xdk_type_base;
extern const XdkTypeInfo xdk_type_display;
extern const XdkTypeInfo xdk_type_screen;
extern const XdkTypeInfo xdk_type_window;
extern const XdkTypeInfo xdk_type_gc;
extern const XdkTypeInfo xdk_type_visual;

const XdkTypeInfo const * xdk_type_infos[XDK_TYPE_MAX] = {
	& xdk_type_invalid,
	& xdk_type_base,
	& xdk_type_display,
	& xdk_type_screen,
	& xdk_type_window,
	& xdk_type_gc,
	& xdk_type_visual,
};
