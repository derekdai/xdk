#ifndef __XDK_BASE_H_
#define __XDK_BASE_H_

#include <glib.h>

G_BEGIN_DECLS

#define XDK_BASE(o)				(XDK_CAST(o, XdkBase, XDK_TYPE_BASE))

typedef struct _XdkBase XdkBase;

gpointer xdk_base_new(XdkType type);

gpointer xdk_base_ref(gpointer base);

void xdk_base_unref(gpointer base);

G_END_DECLS

#endif /* __XDK_BASE_H_ */
