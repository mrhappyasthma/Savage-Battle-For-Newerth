#define OPENSSL_DISABLE_OLD_DES_SUPPORT
#include "openssl/ssl.h"

void	Hash_Init();
int 	Hash_File(file_t *file, unsigned char hash_value[MAX_HASH_SIZE]);
int 	Hash_Filename(char *filename, unsigned char hash_value[MAX_HASH_SIZE]);
int 	Hash_FilenameAbsolute(char *filename, unsigned char hash_value[EVP_MAX_MD_SIZE]);
int     Hash_String(char *string, unsigned char hash_value[EVP_MAX_MD_SIZE]);

bool    Hash_StartHash();
int     Hash_AddData(char *data, int len);
int     Hash_EndHash(unsigned char hash_value[EVP_MAX_MD_SIZE]);
