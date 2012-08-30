#ifndef __XDK_TYPE_REGISTRY_H_
#define __XDK_TYPE_REGISTRY_H_

#include <glib.h>

G_BEGIN_DECLS

enum _XdkType
{
	XDK_TYPE_INVALID,
	XDK_TYPE_BASE,
	XDK_TYPE_DISPLAY,
	XDK_TYPE_SCREEN,
	XDK_TYPE_WINDOW,
	XDK_TYPE_GC,
	XDK_TYPE_VSIUAL,
	XDK_TYPE_MAX
};

G_END_DECLS

#endif /* __XDK_TYPE_REGISTRY_H_ */