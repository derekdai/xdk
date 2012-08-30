#ifndef __XDK_VISUAL_H_
#define __XDK_VISUAL_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define XDK_VISUAL(o)			(XDK_CAST(o, XdkVisual, XDK_TYPE_VISUAL))

typedef struct _XdkVisual XdkVisual;

struct _XdkVisual
{
	GObject base;
};

G_END_DECLS

#endif /* __XDK_VISUAL_H_ */
