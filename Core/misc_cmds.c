// (C) 2003 S2 Games

// misc_cmds.c

// miscellaneous console commands


#include "core.h"

cvar_t screenshotFormat = { "screenshotFormat", "jpg" };
cvar_t screenshotQuality = { "screenshotQuality", "90" };

void	Dir_DirCallback(const char *filename, void *userdata)
{
	Console_Printf("%s <directory>\n", filename);
}
void	Dir_FileCallback(const char *filename, void *userdata)
{
	Console_Printf("%s\n", filename);
}

void	Dir_Cmd(int argc, char *argv[])
{
	bool recurse = false;

	if (argc > 1)
		recurse = true;

	if (argc)
	{		
		Console_Printf("Directory of %s\n------\n\n", argv[0]);	
		System_Dir(argv[0], "*", recurse, Dir_DirCallback, Dir_FileCallback, NULL);		
	}
	else
	{
		Console_Printf("Directory of ./*\n------\n\n");
		System_Dir(".", "*", recurse, Dir_DirCallback, Dir_FileCallback, NULL);
	}
}


void	Echo_Cmd(int argc, char *argv[])
{
	char s[1024];

	ConcatArgs(argv, argc, s);

	Console_Printf("%s\n", s);
}

void SaveScreenShot_Cmd(int argc, char *argv[])
{
	char filename[256];
	bitmap_t screenshot;

    if (!argc)
	{
		File_GetNextFileIncrement(5, "screenshots/shot", screenshotFormat.string, filename, 255);
	}
	else	
		strncpy(filename, argv[0], 255);

	System_CreateDir(Filename_GetDir(filename));

	Vid_GetFrameBuffer(&screenshot);
	if (stricmp(screenshotFormat.string, "png") == 0)
		Bitmap_WritePNG(filename, &screenshot);
	else
		Bitmap_WriteJPEG(filename, &screenshot, screenshotQuality.integer);
	Bitmap_Free(&screenshot);

	Console_Printf("Screenshot saved to %s\n", filename);
}

void LineBoxTest_Cmd(int argc, char *argv[])
{
	vec3_t bmin,bmax;
	vec3_t p1,p2;
	int n;
	double timestart,time1,time2;
	float fraction;
	vec3_t out2;
	bool ok1,ok2;

	timestart = System_GetPerfCounter();

	srand(12345);
	for (n=0; n<10000000; n++)
	{	
		M_ClearBounds(bmin,bmax);
		M_AddPointToBounds(vec3(-1000,-1000,-1000), bmin, bmax);
		M_AddPointToBounds(vec3(1000,1000,1000), bmin, bmax);

		p1[0] = M_Randnum(-10000, 10000);
		p1[1] = M_Randnum(-10000, 10000);
		p1[2] = M_Randnum(-10000, 10000);
		p2[0] = M_Randnum(-10000, 10000);
		p2[1] = M_Randnum(-10000, 10000);
		p2[2] = M_Randnum(-10000, 10000);

		ok1 = M_LineBoxIntersect3d(p1, p2, bmin, bmax, &fraction);
	}

	time1 = System_GetPerfCounter() - timestart;
	timestart = System_GetPerfCounter();

	srand(12345);
	for (n=0; n<10000000; n++)
	{
		M_ClearBounds(bmin,bmax);
		M_AddPointToBounds(vec3(-1000,-1000,-1000), bmin, bmax);
		M_AddPointToBounds(vec3(1000,1000,1000), bmin, bmax);

		p1[0] = M_Randnum(-10000, 10000);
		p1[1] = M_Randnum(-10000, 10000);
		p1[2] = M_Randnum(-10000, 10000);
		p2[0] = M_Randnum(-10000, 10000);
		p2[1] = M_Randnum(-10000, 10000);
		p2[2] = M_Randnum(-10000, 10000);

	//	M_SubVec3(p2, p1, dir);
	//	M_Normalize(dir);

		ok2 = M_RayBoxIntersect(p1, p2, bmin, bmax, out2);
	}

	time2 = System_GetPerfCounter() - timestart;

	if (time1 < time2)
	{
		Console_Printf("My function is faster by %f, taking %f\n",time2-time1,time1);
	}
	else
	{
		Console_Printf("Woo's function is faster by %f, taking %f\n",time1-time2,time2);
	}	
}

void MiscCmds_Register()
{
	Cmd_Register("dir", Dir_Cmd);
	Cmd_Register("echo", Echo_Cmd);
	Cmd_Register("screenshot", SaveScreenShot_Cmd);
	Cmd_Register("lineboxtest", LineBoxTest_Cmd);

	Cvar_Register(&screenshotFormat);
	Cvar_Register(&screenshotQuality);
}

