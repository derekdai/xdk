/**
 * The best way to get yourself understand Xlib, see
 *   http://tronche.com/gui/x/xlib
 */

#ifndef __XDK_H_
#define __XDK_H_

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

G_BEGIN_DECLS

#define XDK_GC(o)				(XDK_CAST(o, XdkGc, XDK_TYPE_GC))
#define XDK_VISUAL(o)			(XDK_CAST(o, XdkVisual, XDK_TYPE_VISUAL))

typedef struct _XdkBase XdkBase;

typedef struct _XdkDisplay XdkDisplay;

typedef struct _XdkScreen XdkScreen;

typedef struct _XdkWindow XdkWindow;

typedef struct _XdkGc XdkGc;

typedef struct _XdkVisual XdkVisual;

#include "xdk-type.h"
#include "xdk-type-registry.h"
#include "xdk-base.h"
#include "xdk-display.h"
#include "xdk-screen.h"
#include "xdk-window.h"

gboolean xdk_init(int * argc, char ** args[]);

G_END_DECLS

#endif /* __XDK_H_ */
