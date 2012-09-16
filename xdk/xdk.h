/**
 * The best way to get yourself understand Xlib, see
 *   http://tronche.com/gui/x/xlib
 */

#ifndef __XDK_H_
#define __XDK_H_

#include <glib-object.h>
#include <X11/Xlib.h>
#include "xdk-types.h"
#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-window.h"
#include "xdk-gc.h"
#include "xdk-visual.h"

G_BEGIN_DECLS

void xdk_disable_event_retrieval();

gboolean xdk_is_event_retrieval_disabled();

void xdk_set_xdisplay(Display * display);

Display * xdk_get_xdisplay();

void xdk_init(int * argc, char ** args[]);

void xdk_main();

void xdk_main_quit();

G_END_DECLS

#endif /* __XDK_H_ */
