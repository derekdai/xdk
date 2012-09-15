#ifndef __XDK_SCREEN_PRIVATE_H_
#define __XDK_SCREEN_PRIVATE_H_

#include <glib.h>

G_BEGIN_DECLS

void _xdk_screen_set_display(XdkScreen * self, XdkDisplay * display);

void _xdk_screen_set_peer(XdkScreen * self, Screen * peer);

void _xdk_screen_prepare_root_window(XdkScreen * self);

G_END_DECLS

#endif /* __XDK_SCREEN_PRIVATE_H_ */
