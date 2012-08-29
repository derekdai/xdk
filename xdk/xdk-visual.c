#include "xdk-type.h"
#include "xdk-base-private.h"

struct _XdkVisual
{
	XdkBase parent;
};

const XdkTypeInfo xdk_type_visual = {
	XDK_TYPE_BASE,
	"XdkVisual",
	0,
	NULL,
	NULL
};
