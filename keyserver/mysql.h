#include "../Core/savage_types.h"
#include "../masterserver/server.h"
#include "../masterserver/common.h"
#include "../masterserver/mysql.h"

typedef enum
{
	BAD_LOGIN,
	BAD_CDKEY,
	ALREADY_PLAYING,
	EXPIRED_BUILD,
	COULDNT_CONNECT_TO_GAME,
	INVALID_KEYVALUE
} auth_failure_reasons;

void	db_init();
void    *db_connect();
void    db_disconnect(void *handle);

unsigned int    db_get_clan(void *handle, unsigned int user_id);

unsigned int 	db_get_game_id(void *handle, char *ip_address, int port);
bool    		db_check_game_cookie_exists (void *handle, unsigned int game_id, char *cookie);

unsigned int    db_create_user(void *handle, char *username, char *password, char *cdkey);
unsigned int    db_check_user_pass(void *handle, char *username, char *password);
unsigned int    db_check_user_md5pass(void *handle, char *username, char *md5password);
unsigned int 	get_last_game(void *handle, unsigned int user_id, char server[], int *port);

bool    db_add_player_to_game (void *handle, unsigned int user_id, char *server, int port, char *cookie, char *cdkey);

bool    db_log_connection (void *handle, unsigned int user_id, char *ip, char *cdkey, char *server, int port, char *platform, char *build);
bool    db_log_auth_failure (void *handle, char *username, char *password, char *ip, char *cdkey, char *server, int port, char *platform, char *build, int reason);

int     db_check_cdkey (void *handle, char *cdkey);
int     db_cdkey_in_use (void *handle, char *cdkey);

bool    db_game_appears_dead (void *handle, unsigned int game_id);

bool	db_register_nick(void *handle, unsigned int user_id, char *nick);

char	*db_get_clan_abbrev(void *handle, unsigned int clan_id);

int     db_count_instances_playing(void *handle, unsigned int user_id);
