#include "server_game.h"
#include "sv_aiutil.h"

int SV_AI_RandomTime(int lo, int hi)
{
	//don't ever let a <=0 modulus happen
	if (hi <= lo)
		hi = lo + 1;

	return ((rand() % (hi-lo)) + lo);
}
