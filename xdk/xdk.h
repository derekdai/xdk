/**
 * The best way to get yourself understand Xlib, see
 *   http://tronche.com/gui/x/xlib
 */

#ifndef __XDK_H_
#define __XDK_H_

#include <glib.h>

G_BEGIN_DECLS

#include "xdk-type.h"
#include "xdk-type-registry.h"
#include "xdk-base.h"
#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-window.h"
#include "xdk-gc.h"
#include "xdk-visual.h"

gboolean xdk_init(int * argc, char ** args[]);

G_END_DECLS

#endif /* __XDK_H_ */
