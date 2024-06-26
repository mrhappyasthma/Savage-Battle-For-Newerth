
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	printf("Checking for game updates...\n");
#ifdef _WIN32
	return system("updater\\perl.exe updater\\winupdate.pl");
#else
	//only run the game if the patch was successful
	return system("updater/perl updater/linuxupdate.pl");
#endif
}
