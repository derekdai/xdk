#include "xdk-types.h"
#include "xdk-display.h"

static const char * xdk_error_names[] = {
	"XDK_ERROR_SUCCESS (Success)",
	"XDK_ERROR_REQUEST (BadRequest)",
	"XDK_ERROR_VALUE (BadValue)",
	"XDK_ERROR_WINDOW (BadWindow)",
	"XDK_ERROR_PIXMAP (BadPixmap)",
	"XDK_ERROR_ATOM (BadAtom)",
	"XDK_ERROR_CUSOR (BadCursor)",
	"XDK_ERROR_FONT (BadFont)",
	"XDK_ERROR_MATCH (BadMatch)",
	"XDK_ERROR_DRAWABLE (BadDrawable)",
	"XDK_ERROR_ACCESS (BadAccess)",
	"XDK_ERROR_ALLOC (BadAlloc)",
	"XDK_ERROR_COLOR (BadColor)",
	"XDK_ERROR_GC (BadGC)",
	"XDK_ERROR_ID_CHOICE (BadIDChoice)",
	"XDK_ERROR_NAME (BadName)",
	"XDK_ERROR_LENGTH (BadLength)",
	"XDK_ERROR_IMPLEMENTAION (BadImplementation)",
};

/**
 * Copy from X11/Xproto.h
 */
static const char * x_request_names[] = {
	"Unknown(0)",
	"CreateWindow(1)",
	"ChangeWindowAttributes(2)",
	"GetWindowAttributes(3)",
	"DestroyWindow(4)",
	"DestroySubwindows(5)",
	"ChangeSaveSet(6)",
	"ReparentWindow(7)",
	"MapWindow(8)",
	"MapSubwindows(9)",
	"UnmapWindow(10)",
	"UnmapSubwindows(11)",
	"ConfigureWindow(12)",
	"CirculateWindow(13)",
	"GetGeometry(14)",
	"QueryTree(15)",
	"InternAtom(16)",
	"GetAtomName(17)",
	"ChangeProperty(18)",
	"DeleteProperty(19)",
	"GetProperty(20)",
	"ListProperties(21)",
	"SetSelectionOwner(22)",
	"GetSelectionOwner(23)",
	"ConvertSelection(24)",
	"SendEvent(25)",
	"GrabPointer(26)",
	"UngrabPointer(27)",
	"GrabButton(28)",
	"UngrabButton(29)",
	"ChangeActivePointerGrab(30)",
	"GrabKeyboard(31)",
	"UngrabKeyboard(32)",
	"GrabKey(33)",
	"UngrabKey(34)",
	"AllowEvents(35)",
	"GrabServer(36)",
	"UngrabServer(37)",
	"QueryPointer(38)",
	"GetMotionEvents(39)",
	"TranslateCoords(40)",
	"WarpPointer(41)",
	"SetInputFocus(42)",
	"GetInputFocus(43)",
	"QueryKeymap(44)",
	"OpenFont(45)",
	"CloseFont(46)",
	"QueryFont(47)",
	"QueryTextExtents(48)",
	"ListFonts(49)",
	"ListFontsWithInfo(50)",
	"SetFontPath(51)",
	"GetFontPath(52)",
	"CreatePixmap(53)",
	"FreePixmap(54)",
	"CreateGC(55)",
	"ChangeGC(56)",
	"CopyGC(57)",
	"SetDashes(58)",
	"SetClipRectangles(59)",
	"FreeGC(60)",
	"ClearArea(61)",
	"CopyArea(62)",
	"CopyPlane(63)",
	"PolyPoint(64)",
	"PolyLine(65)",
	"PolySegment(66)",
	"PolyRectangle(67)",
	"PolyArc(68)",
	"FillPoly(69)",
	"PolyFillRectangle(70)",
	"PolyFillArc(71)",
	"PutImage(72)",
	"GetImage(73)",
	"PolyText8(74)",
	"PolyText16(75)",
	"ImageText8(76)",
	"ImageText16(77)",
	"CreateColormap(78)",
	"FreeColormap(79)",
	"CopyColormapAndFree(80)",
	"InstallColormap(81)",
	"UninstallColormap(82)",
	"ListInstalledColormaps(83)",
	"AllocColor(84)",
	"AllocNamedColor(85)",
	"AllocColorCells(86)",
	"AllocColorPlanes(87)",
	"FreeColors(88)",
	"StoreColors(89)",
	"StoreNamedColor(90)",
	"QueryColors(91)",
	"LookupColor(92)",
	"CreateCursor(93)",
	"CreateGlyphCursor(94)",
	"FreeCursor(95)",
	"RecolorCursor(96)",
	"QueryBestSize(97)",
	"QueryExtension(98)",
	"ListExtensions(99)",
	"ChangeKeyboardMapping(100)",
	"GetKeyboardMapping(101)",
	"ChangeKeyboardControl(102)",
	"GetKeyboardControl(103)",
	"Bell(104)",
	"ChangePointerControl(105)",
	"GetPointerControl(106)",
	"SetScreenSaver(107)",
	"GetScreenSaver(108)",
	"ChangeHosts(109)",
	"ListHosts(110)",
	"SetAccessControl(111)",
	"SetCloseDownMode(112)",
	"KillClient(113)",
	"RotateProperties(114)",
	"ForceScreenSaver(115)",
	"SetPointerMapping(116)",
	"GetPointerMapping(117)",
	"SetModifierMapping(118)",
	"GetModifierMapping(119)",
	"Unknown(120)",
	"Unknown(121)",
	"Unknown(122)",
	"Unknown(123)",
	"Unknown(124)",
	"Unknown(125)",
	"Unknown(126)",
	"NoOperation(127)",
};

static char xdk_error_buf[128];

static XEvent * x_event_copy(XEvent * event)
{
	g_return_val_if_fail(event, NULL);
	
	return g_slice_dup(XEvent, event);
}

static void x_event_free(XEvent * event)
{
	g_return_if_fail(event);
	
	g_slice_free(XEvent, event);
}

GType x_event_get_type()
{
	static volatile gsize type_id = 0;
	if(g_once_init_enter(& type_id)) {
		GType tmp = g_boxed_type_register_static(
			"XEvent",
			(GBoxedCopyFunc) x_event_copy,
			(GBoxedFreeFunc) x_event_free);
		g_once_init_leave(& type_id, tmp);
	}
	
	return type_id;
}

const char * xdk_error_type_to_string(XdkErrorType type)
{
	XdkDisplay * display = xdk_display_get_default();
	XGetErrorText(
		xdk_display_get_peer(display),
		type,
		xdk_error_buf, sizeof(xdk_error_buf));
		
	return xdk_error_buf;
}

GQuark xdk_error_quark()
{
	static GQuark error_quark = 0;
	if(! error_quark) {
		error_quark = g_quark_from_static_string("XdkError");
	}
	
	return error_quark;
}

const char * request_code_to_string(gint request_code)
{
	if(request_code < 0 || request_code >= G_N_ELEMENTS(x_request_names)) {
		return "Unknown";
	}
	
	return x_request_names[request_code];
}

GError * xdk_error_new(XErrorEvent * error)
{
	g_return_val_if_fail(error, NULL);
	
	GError * gerror = NULL;
	if(XDK_ERROR_SUCCESS != error->error_code) {
		gerror = g_error_new(
			XDK_ERROR,
			error->error_code,
			"%s [display=%s, serial=%lu, request_code=%s, minor_code=%u, resourceid=0x%x]",
			xdk_error_type_to_string(error->error_code),
			XDisplayString(error->display),
			error->serial,
			request_code_to_string(error->request_code),
			error->minor_code,
			error->resourceid);
	}
	
	return gerror;
}
