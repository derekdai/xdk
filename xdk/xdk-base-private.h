#ifndef __XDK_BASE_PRIVATE_H_
#define __XDK_BASE_PRIVATE_H_

#include <glib.h>
#include "xdk-type.h"
#include "xdk-base.h"

G_BEGIN_DECLS

struct _XdkBase
{
	XdkType type;
	
	int refcount;
};

G_END_DECLS

#endif /* __XDK_BASE_PRIVATE_H_ */
