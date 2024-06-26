// (C) 2003 S2 Games

// input_sdl.c

// sdl specific input (mouse/keyboard) code

#include "SDL/SDL.h"

#include "core.h"

extern bool hasMouseFocus;

bool Input_SDL_SetMouseXY(int x, int y);

input_driver_t input_driver =
{
	"Default",
	Input_SDL_SetMouseXY
};

bool Input_SDL_SetMouseXY(int x, int y)
{
	if (hasMouseFocus)
	{
		SDL_WarpMouse(x, y);
		return true;
	}
	return false;
}

