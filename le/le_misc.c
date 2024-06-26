// le_misc.c

#include "le.h"

double perf_counts[PERF_NUMTYPES];

void	Perf_Count(int perftype, double amount)
{
	if (perftype < 0 || perftype >= PERF_NUMTYPES)
		return;

	perf_counts[perftype] += amount * 1000;
}

void	Perf_Clear()
{
	memset(perf_counts, 0, sizeof(perf_counts));
}

void	Perf_Print()
{
	if (!showGamePerf.integer)
		return;

	core.Console_Printf("==== Editor Frame ====\n\n");
	core.Console_Printf("Terrain op ms:            %.2f\n", perf_counts[PERF_TERRAINOP]);
	core.Console_Printf("Draw frame ms:            %.2f\n", perf_counts[PERF_DRAWFRAME]);
	core.Console_Printf("  Clear ms:               %.2f\n", perf_counts[PERF_CLEARFRAME]);
	core.Console_Printf("  Setup ms:               %.2f\n", perf_counts[PERF_SETUP]);
	core.Console_Printf("  Sky ms:                 %.2f\n", perf_counts[PERF_DRAWSKY]);
	core.Console_Printf("  Occluders ms:           %.2f\n", perf_counts[PERF_OCCLUDERS]);	
	core.Console_Printf("  Time of day ms:         %.2f\n", perf_counts[PERF_TOD]);
	core.Console_Printf("  Objects ms:             %.2f\n", perf_counts[PERF_DRAWOBJECTS]);
	core.Console_Printf("  Render ms:              %.2f\n", perf_counts[PERF_RENDERSCENE]);
	core.Console_Printf("\n");
}