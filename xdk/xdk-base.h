#ifndef __XDK_BASE_H_
#define __XDK_BASE_H_

#define XDK_CAST(o, c, x)		((c *) o)
#define XDK_BASE(o)				(XDK_CAST(o, XdkBase, XDK_TYPE_BASE))

gpointer xdk_base_new(XdkType type);

gpointer xdk_base_ref(gpointer base);

void xdk_base_unref(gpointer base);

#endif /* __XDK_BASE_H_ */
