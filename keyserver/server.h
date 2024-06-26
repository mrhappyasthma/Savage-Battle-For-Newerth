#include "auth_common.h"

#define MAX_QUEUE_LENGTH    10000

void	server_init();
int     server_listen();
bool    process_auth_request(char *buf, int len, char *remote_ip, char *cookie, int cookielen, int *user_id, char **reason);
