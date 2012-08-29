#ifndef __XDK_TYPE_H_
#define __XDK_TYPE_H_

#include <glib.h>
#include "xdk-type-registry.h"

G_BEGIN_DECLS

#define XDK_CAST(o, c, x)		((c *) o)

typedef struct _XdkTypeInfo XdkTypeInfo;

typedef enum _XdkType XdkType;

typedef gboolean (* XdkInitFunc)(gpointer base);

typedef void (* XdkDestroyFunc)(gpointer base);

struct _XdkTypeInfo
{
	guint parent;
	const char * name;
	gsize size;
	XdkInitFunc init_func;
	XdkDestroyFunc destroy_func;
};

const char * xdk_type_get_name(XdkType type);

XdkInitFunc xdk_type_get_init_func(XdkType type);

XdkDestroyFunc xdk_type_get_destroy_func(XdkType type);

XdkType xdk_type_get_parent(XdkType type);

G_END_DECLS

#endif /* __XDK_TYPE_H_ */
