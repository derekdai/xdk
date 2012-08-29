#ifndef __XDK_GC_H_
#define __XDK_GC_H_

#include <glib.h>
#include "xdk-type.h"

G_BEGIN_DECLS

#define XDK_GC(o)				(XDK_CAST(o, XdkGc, XDK_TYPE_GC))

typedef struct _XdkGc XdkGc;

G_END_DECLS

#endif /* __XDK_GC_H_ */
