#define MAX_BANNED_USERS 500

void	Bans_Init();
void	Bans_Shutdown();
void	Bans_Load(char *filename);
void	Bans_Save(char *filename);
void    Bans_Frame();

void    Bans_Add(char *name, int expirationTime);

char 	*Ban_IsIPBanned(char *ip_address);
bool    Ban_IsUserIdBanned(int user_id);
