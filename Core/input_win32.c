// (C) 2003 S2 Games

// input_win32.c

// win32 specific input (mouse/keyboard) code


//don't even attempt to compile if this is not win32
#ifdef _WIN32

#include "core.h"

//sys_win32.h
HWND	System_Win32_GetHWnd();

bool Input_Win32_SetMouseXY(int x, int y);

input_driver_t input_driver =
{
	"Default",
	Input_Win32_SetMouseXY
};

bool	Input_Win32_CheckCursorInClientArea(void)
{
	RECT winrec;
	POINT point;
	int xoff = 0, yoff = 0;

	GetCursorPos(&point);
	Vid_GetClientOffset(&xoff, &yoff);
	GetWindowRect(System_Win32_GetHWnd(), &winrec);

	if (point.x < (winrec.left + xoff) ||
		point.x > (winrec.left + xoff + winrec.right) ||
		point.y < (winrec.top + yoff) ||
		point.y > (winrec.top + yoff + winrec.bottom) )
	{
		return false;
	}

	return true;
}

bool	Input_Win32_SetMouseXY(int x, int y)
{
	RECT winrec;
	int xoff = 0, yoff = 0;

	Vid_GetClientOffset(&xoff, &yoff);
	GetWindowRect(System_Win32_GetHWnd(), &winrec);
	//only set the mouse position if we have mouse focus
	//if (GetCapture() == System_Win32_GetHWnd())
		SetCursorPos(winrec.left + xoff + x, winrec.top + yoff + y);

	return true;
}

#endif //_WIN32
