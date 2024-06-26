// (C) 2003 S2 Games

// le_pool.h

// simple string and struct allocation from a pre-allocated pool


void	*LE_Alloc(int size);
char	*LE_StrDup(const char *s);
void	LE_Pool_Init();
