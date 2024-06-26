#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32

#define WINDOWS_LEAN_AND_MEAN
#include	<windows.h>

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	DWORD				tstat;
	struct _stat		fstat;
	int					pass = 0;
	char				cmdbuffer[1024];

	do
	{
		if (pass > 0)
		{
			remove("updater.exe");
			rename("updater_new.exe", "updater.exe");
		}

		//run the patcher
		GetStartupInfo(&si);
		CreateProcess(NULL, "updater.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &tstat);
		pass++;
	}
	while (_stat("updater_new.exe", &fstat) == 0 && tstat == 0);

	//if (tstat == 0)
	{
#ifdef _DEBUG
		strcpy(cmdbuffer, "silverback_debug.exe ");
#else	//_DEBUG
		strcpy(cmdbuffer, "silverback.exe ");
#endif	//_DEBUG

		strncat(cmdbuffer, lpCmdLine, 1023 - strlen(cmdbuffer));
		CreateProcess(NULL, cmdbuffer, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}
}

#else	// the unix code

#include <string.h>

int file_exists(char *filename)
{
	FILE *f;

	f = fopen(filename, "r");
	if (f)
	{
		fclose(f);
		return 1;
	}
	return 0;
}

void 	set_executable(char *filename)
{
	struct stat stats;

	if (stat(filename, &stats) == 0)
	{
		if (stats.st_mode & S_IXUSR 
			|| stats.st_mode & S_IXOTH)
			return;
		if (chmod(filename, stats.st_mode | S_IXUSR))
		{
			fprintf(stderr, "error trying to set %s to be executable\n", filename);
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int done = 0;
	
	while (!done)
	{
		while (file_exists("update.new"))
		{
			ret = remove("update");
			if (ret)
			{
				fprintf(stderr, "Unable to delete file update.  You must have write permission to this directory!\n");
				exit(1);
			}
			ret = rename("update.new", "update");
			if (ret)
			{
				fprintf(stderr, "Unable to rename update_new to update!  Check directory and file permissions?\n");
				exit(1);
			}
		}
		//look for malicious attempts if we can!
		if (file_exists("runme"))
			remove("runme");
		
		set_executable("update");
		ret = system("LD_LIBRARY_PATH=`pwd`/libs:$LD_LIBRARY_PATH ./update");
		if (file_exists("runme"))
		{
			set_executable("runme");
			system("./runme");
			remove("runme");
		}
		if (file_exists("savage.bin.new"))
		{
			remove("savage.bin");
			rename("savage.bin.new", "savage.bin");
			set_executable("savage.bin");
		}
		if (!file_exists("update_new"))
			done = 1;
	}
	//only run the game if the patch was successful
	if (ret)
	{
		fprintf(stderr, "There was an error (error %i - %s) running the updater!  bailing out\n", ret, strerror(ret));
		exit(1);
	}
	ret = system("LD_LIBRARY_PATH=`pwd`/libs:$LD_LIBRARY_PATH ./silverback.bin set mod game");
	return ret;
}
#endif	//_WIN32
