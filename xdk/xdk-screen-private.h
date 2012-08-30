#ifndef __XDK_SCREEN_PRIVATE_H_
#define __XDK_SCREEN_PRIVATE_H_

#include <glib.h>
#include "xdk-screen.h"

G_BEGIN_DECLS

void xdk_screen_set_display(XdkScreen * self, XdkDisplay * display);

void xdk_screen_set_peer(XdkScreen * self, Screen * peer);

G_END_DECLS

#endif /* __XDK_SCREEN_PRIVATE_H_ */
