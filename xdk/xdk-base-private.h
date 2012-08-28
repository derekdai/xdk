#ifndef __XDK_BASE_PRIVATE_H_
#define __XDK_BASE_PRIVATE_H_

struct _XdkBase
{
	XdkType type;
	
	int refcount;
};

#endif /* __XDK_BASE_PRIVATE_H_ */
