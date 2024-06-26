// (C) 2003 S2 Games

// html.h

// HTML code functionality

#define HTML_HEADER "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 " \
	"Transitional//EN\">\n<html><STYLE><!--\nBODY { font-family: " \
	" Verdana,Arial,Helvetica; }\nTABLE { font-size: 10pt; } \n" \
	".detailed { font-size: 9pt } \n" \
	".title { font-size: 11pt } \n" \
	".header { font-size: 10pt } \n" \
	"A:hover { text-decoration: underline; color: red } \n" \
	"A { text-decoration: none; color: navy } \n" \
	"//--></STYLE>\n" 

#define JS_PANELSCRIPT "<SCRIPT TYPE=\"text/javascript\">\n" \
	"<!--\n" \
	"function togglePanel(idName) {\n" \
	"	obj = document.all.item(idName);\n" \
	"	aObj = document.all.item(idName + \"_link\");\n" \
	"	aObj.style.fontWeight = (aObj.style.fontWeight == \"normal\") ? \"bold\": \"normal\";\n" \
	"	if (obj.style.visibility == \"visible\") {\n" \
	"		obj.style.visibility = \"hidden\";\n" \
	"		obj.style.display = \"none\";\n" \
	"	} else {\n" \
	"		obj.style.visibility = \"visible\";\n" \
	"		obj.style.display = \"block\";\n" \
	"	}\n" \
	"}\n" \
	"//-->\n" \
	"</SCRIPT>\n"
  
#define JS_POPUPWINDOW "<SCRIPT TYPE=\"text/javascript\">\n" \
	"<!--\n" \
	"function openWin(URL, W, H, winName) {\n" \
	"	tgaWindow = window.open(URL,winName," \
	"	  \"width=\"+W+\",height=\"+H+\",scrollbars=no,menubar=no,resizable=yes,status=no,toolbar=no\");\n" \
	"}\n" \
	"//-->\n" \
	"</SCRIPT>\n"