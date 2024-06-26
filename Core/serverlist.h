#include "core.h"

//from net_server.v
void	close_socket(int s);

int 	MasterServer_Init();
int     MasterServer_TestFirewall();
int 	MasterServer_GetGameList();
int 	MasterServer_GetGameList_Frame(server_info_t *servers, int max_servers, int *num_servers);
void	MasterServer_SetGameInfo(char *address, int port, int ping, const char *coreInfo, const char *gameInfo);
void    MasterServer_EraseGameInfo(char *address, int port);
bool    MasterServer_GetGameInfo(char *address, int port, int *ping, char **coreInfo, char **gameInfo);

int     MasterServer_FindUsers(char *name);
int     MasterServer_GetUserList_Frame(user_info_t *users, int max_users, int *num_users);

int     MasterServer_GetUserInfo(char *cookie, char *server, int port);
int     MasterServer_GetUserInfo_Frame(char *info, int maxlen);
