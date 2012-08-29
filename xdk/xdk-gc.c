#include "xdk-type.h"
#include "xdk-base-private.h"

struct _XdkGc
{
	XdkBase parent;
};

const XdkTypeInfo xdk_type_gc = {
	XDK_TYPE_BASE,
	"XdkGc",
	0,
	NULL,
	NULL
};
