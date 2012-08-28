#ifndef __XDK_SCREEN_H_
#define __XDK_SCREEN_H_

struct _XdkScreen
{
	XdkBase parent;
	
	XdkDisplay * display;
	
	Screen * peer;
	
	XdkVisual * default_visual;
	
	XdkGc * default_gc;
	
	XdkWindow * root;
};

void xdk_screen_set_peer(XdkScreen * screen, Screen * peer);

#endif /* __XDK_SCREEN_H_ */
