#ifndef __XDK_UTIL_H_
#define __XDK_UTIL_H_

#include <glib.h>
#include <X11/Xlib.h>

G_BEGIN_DECLS

void xdk_util_event_dump(XEvent * event);

char * xdk_util_event_to_string(XEvent * event);

const char * xdk_util_event_get_name(XEvent * event);

void xdk_util_window_dump(Window window);

const char * xdk_util_window_to_string(Window window);

G_END_DECLS

#endif /* __XDK_UTIL_H_ */
