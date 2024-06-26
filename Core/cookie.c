#ifndef EXCLUDE_CORE
#include "core.h"
#else
#include <sys/types.h>
#include <unistd.h>
#include "savage_types.h"
#endif

#ifdef _WIN32
#include <process.h>
#endif

#ifdef INCLUDE_MRANDNUM_DEF
/*
 * simple random number generator
 * randNum - Return a random floating point number such that
 *      (min <= return-value <= max)
 * 32,767 values are possible for any given range.
 */
float M_Randnum (float min, float max)
{
	int r;
	float   x;
	
	r = rand ();
	x = (float)(r & 0x7fff) /
		(float)0x7fff;
	return (x * (max - min) + min);
}
#endif

//generates a small cookie to identify this client from now on
bool	generate_client_cookie(char *cookie, int length)
{
	int i;
	pid_t pid;

	if (length <= 0)
		return false;

#ifdef _WIN32
	pid = _getpid();
#else
	pid = getpid();
#endif
	srand((int)pid + cookie[2]);
	
	for (i = 0; i < length; i++)
	{
		cookie[i] = (int)M_Randnum('0', '~');
		while (cookie[i] == '\''
				|| cookie[i] == '\\')
		{
			cookie[i] = (int)M_Randnum('0', '~');
		}
	}

	//Console_DPrintf("assigning client the cookie '%s'\n", cookie);
	
	return true;
}
