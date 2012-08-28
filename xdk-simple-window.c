#include "xdk.h"

int main(int * argc, char * args[])
{
	if(! xdk_init(NULL, NULL)) {
		g_error("Failed to init XDK");
	}
	
	XdkDisplay * display = xdk_display_get_default();
	
	
	return 0;
}
