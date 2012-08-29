#ifndef __XDK_TYPE_H_
#define __XDK_TYPE_H_

typedef struct _XdkTypeInfo XdkTypeInfo;

typedef enum _XdkType XdkType;

typedef gboolean (* XdkInitFunc)(gpointer base);

typedef void (* XdkDestroyFunc)(gpointer base);

struct _XdkTypeInfo
{
	guint parent;
	const char * name;
	XdkInitFunc init_func;
	XdkDestroyFunc destroy_func;
	gsize size;
};

#endif /* __XDK_TYPE_H_ */
