// (C) 2003 S2 Games

// stringtable.h



#define STRING_TABLE_NAME_LENGTH	64

typedef struct
{
	char *name;
	char *value;
} stringTableEntry_t;

typedef struct
{
	struct media_s	*media;

	char name[STRING_TABLE_NAME_LENGTH];

	stringTableEntry_t *entries;
	int	num_entries;
} stringTable_t;


bool	Str_LoadStringTable(const char *filename, stringTable_t *outTable);
char	*Str_Get(residx_t stringtable, const char *stringname);
