// (C) 2003 S2 Games

// stringtable.c


#include "core.h"



char	*Str_Get(residx_t stringtable, const char *stringname)
{
	int n;
	stringTable_t *table = Res_GetStringTable(stringtable);

	if (!table)
		return "";

	for (n=0; n<table->num_entries; n++)
	{
		if (stricmp(stringname, table->entries[n].name)==0)
		{
			return table->entries[n].value;
		}
	}

	return "";
	//return fmt("STRING_NOT_FOUND:%s", stringname);
}

bool	Str_ParseBuffer(char *buf, int size, stringTable_t *outTable)
{
	int current = 0;
	int num_alloced = 0;
	int n = 0;
	char *linestart;

	while(1)
	{		
		if (num_alloced <= current)
		{
			//alloc some strings
			outTable->entries = Tag_Realloc(outTable->entries, (num_alloced+32) * sizeof(stringTableEntry_t), MEM_STRINGTABLE);

			num_alloced += 32;
		}
		
		while (IsTokenSeparator(buf[n]) && n < size)			//skip leading spaces
			n++;
		if (n>=size)
			break;

		linestart = &buf[n];

		//get eol
		while (n < size && !IsEOL(buf[n]))
			n++;
		if (n>=size)
			break;

		if (linestart[0] == '/' && linestart[1] == '/')			//comment
			continue;

		buf[n] = 0;		//mark end of line with a null char

		outTable->entries[current].name = linestart;
		outTable->entries[current].value = (char *)GetNextWord(linestart);
		
		current++;

		while (!IsTokenSeparator(*linestart))
			linestart++;
		*linestart = 0;	//so string name doesn't have trailing spaces		

		n++;
		if (n>=size)
			break;

	}

	outTable->num_entries = current;

	if (outTable->num_entries == 0)
	{
		Tag_Free(outTable->entries);
		return false;
	}

	return true;
}

bool	Str_LoadStringTable(const char *filename, stringTable_t *outTable)
{
	char strtablename[_MAX_PATH];
	int size;
	char *buf;
	const char *fname;

	memset(outTable, 0, sizeof(stringTable_t));	

	fname = Filename_GetFilename((char *)filename);
	Filename_StripExtension(fname, strtablename);
	strncpySafe(outTable->name, strtablename, STRING_TABLE_NAME_LENGTH);	

	buf = File_LoadIntoBuffer(filename, &size, MEM_STRINGTABLE);
	if (!buf)
		return false;

	if (!Str_ParseBuffer(buf, size, outTable))
		return false;

	//the buffer we just read in is used to generate the pointers for the string table, so don't free it

	return true;
}
