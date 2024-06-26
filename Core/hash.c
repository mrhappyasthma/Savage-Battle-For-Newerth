
#include "core.h"

#define MAX_DATA_READ 65536
#define PREFERRED_HASH "sha1"

//returns the length of the hash
int	Hash_File(file_t *file, unsigned char hash_value[EVP_MAX_MD_SIZE])
{
	EVP_MD_CTX mdctx;
	int md_len, len, i;
	char data[MAX_DATA_READ];
	const EVP_MD *md;

	OpenSSL_add_all_digests();

	md = EVP_get_digestbyname(PREFERRED_HASH);
	
	if(!md) 
	{
		Console_Printf("Unknown message digest %s\n", PREFERRED_HASH);
		return 0;
	}
	
#ifdef EVP_MD_CTX_init
	EVP_MD_CTX_init(&mdctx);
	EVP_DigestInit_ex(&mdctx, md, NULL);
#else
	EVP_DigestInit(&mdctx, md);
#endif
	
	while (!file->eof(file))
	{
		len = file->read(data, 1, MAX_DATA_READ, file);
		EVP_DigestUpdate(&mdctx, data, len);
	}
#ifdef EVP_DigestFinal_ex
	EVP_DigestFinal_ex(&mdctx, hash_value, &md_len);
	EVP_MD_CTX_cleanup(&mdctx);
#else
	EVP_DigestFinal(&mdctx, hash_value, &md_len);
#endif

	Console_DPrintf("Hash is: ");
	for(i = 0; i < md_len; i++) 
		Console_DPrintf("%02x", hash_value[i]);
	Console_DPrintf("\n");

	/*
	Console_Printf("Hash (4 at a time) is: ");
	for(i = 0; i < md_len/4; i++) 
		Console_Printf("%08x", SwapEndian(*(int *)&hash_value[i*4]));
	Console_Printf("\n");
	*/
	
	return md_len;
}

int	Hash_FilenameAbsolute(char *filename, unsigned char hash_value[EVP_MAX_MD_SIZE])
{
	file_t *f;
	int ret;

	f = File_OpenAbsolute(filename, "rb");
	if (!f)
	{
		Console_DPrintf("hash error: %s not found\n", filename);
		return 0;
	}
	
	Console_DPrintf("hashing file %s\n", filename);
	
	ret = Hash_File(f, hash_value);
	
	File_Close(f);

	return ret;
}
	
int	Hash_Filename(char *filename, unsigned char hash_value[EVP_MAX_MD_SIZE])
{
	file_t *f;
	int ret;

	f = File_Open(filename, "rb");
	if (!f)
	{
		Console_DPrintf("hash error: %s not found\n", filename);
		return 0;
	}
	
	Console_DPrintf("hashing file %s\n", filename);
	
	ret = Hash_File(f, hash_value);
	
	File_Close(f);

	return ret;
}

void	Hash_File_cmd(int argc, char *argv[])
{
	file_t *file;
	int hash_len, i;
	unsigned char hash[EVP_MAX_MD_SIZE];

	if (argc && File_Exists(argv[0]))
	{
		file = File_Open(argv[0], "rb");
		hash_len = Hash_File(file, hash);
		File_Close(file);

		Console_DPrintf("Hash value is: ");
		for(i = 0; i < hash_len; i++) Console_DPrintf("%02x", hash[i]);
		Console_DPrintf("\n");
	}
}

int		Hash_String(char *string, unsigned char hash_value[EVP_MAX_MD_SIZE])
{
	EVP_MD_CTX mdctx;
	const EVP_MD *md;
	int md_len;
	
	OpenSSL_add_all_digests();
	
	md = EVP_get_digestbyname(PREFERRED_HASH);
	
	if(!md) {
		Console_DPrintf("Unknown message digest %s\n", PREFERRED_HASH);
		return 0;
	}
	
#ifdef EVP_MD_CTX_init
	EVP_MD_CTX_init(&mdctx);
	EVP_DigestInit_ex(&mdctx, md, NULL);
#else
	EVP_DigestInit(&mdctx, md);
#endif
	
	EVP_DigestUpdate(&mdctx, string, strlen(string));

#ifdef EVP_DigestFinal_ex
	EVP_DigestFinal_ex(&mdctx, hash_value, &md_len);
	EVP_MD_CTX_cleanup(&mdctx);
#else
	EVP_DigestFinal(&mdctx, hash_value, &md_len);
#endif

	return md_len;
}

static EVP_MD_CTX mdctx;
static const EVP_MD *md;
static int md_len;

bool		Hash_StartHash()
{
	OpenSSL_add_all_digests();
	
	md = EVP_get_digestbyname(PREFERRED_HASH);
	
	if(!md) {
		Console_DPrintf("Unknown message digest %s\n", PREFERRED_HASH);
		return false;
	}
	
#ifdef EVP_MD_CTX_init
	EVP_MD_CTX_init(&mdctx);
	EVP_DigestInit_ex(&mdctx, md, NULL);
#else
	EVP_DigestInit(&mdctx, md);
#endif

	return true;
}

int		Hash_AddData(char *data, int len)
{
	EVP_DigestUpdate(&mdctx, data, len);
	return len;
}

int		Hash_EndHash(unsigned char hash_value[EVP_MAX_MD_SIZE])
{

#ifdef EVP_DigestFinal_ex
	EVP_DigestFinal_ex(&mdctx, hash_value, &md_len);
	EVP_MD_CTX_cleanup(&mdctx);
#else
	EVP_DigestFinal(&mdctx, hash_value, &md_len);
#endif

	return md_len;
}

void	Hash_String_cmd(int argc, char *argv[])
{
	unsigned char md_value[EVP_MAX_MD_SIZE];
	int md_len;
	int i;

	if (!argc)
		return;
	
	md_len = Hash_String(argv[0], md_value);
	
	Console_Printf("Digest is: ");
	for(i = 0; i < md_len; i++) Console_Printf("%02x", md_value[i]);
	Console_Printf("\n");
}

void	Hash_Init()
{
	Cmd_Register("hashString", Hash_String_cmd);
	Cmd_Register("hashFile", Hash_File_cmd);
}
