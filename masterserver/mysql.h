void    *db_connect();
void    db_disconnect(void *handle);
bool    db_get_all_current_games(void *handle, int flags, server_info_t *servers, int max_servers);
bool    db_find_users(void *handle, int flags, char *name, user_info_t *users, int max_users);

unsigned int 	db_get_game_id(void *handle, char *ip_address, int port);
unsigned int    db_get_player_id(void *handle, unsigned int game_id, char *cookie);
unsigned int    db_get_game_user_id(void *handle, unsigned int game_id, char *cookie);
unsigned int    db_get_game_cdkey_id(void *handle, unsigned int game_id, char *cookie);

//stuff for the heartbeat server
bool    db_insert_game_info(void *handle, char *ip_address, int port);
bool    db_update_game_info(void *handle, char *ip_address, int port, int numPlayers, char playerCookies[][]);
bool    db_update_game_info_ex(void *handle, unsigned int game_id, char *map, char *team1_race, char *team2_race, int winningTeam);
bool	db_update_lastheartbeat(void *handle, char *ip_address, int port);
void    db_clear_dead_games(void *handle, int timeout);
bool    db_update_player_stat(void *handle, char *cookie, unsigned int game_id, char *stat_string, char *value);
void    db_mark_game_complete(void *handle, unsigned int game_id, char *reason);
void    db_migrate_players(void *handle, unsigned int old_game_id, unsigned int new_game_id);
bool    db_delete_game_info(void *handle, unsigned int game_id);

bool    db_get_user_info_statestring(void *handle, unsigned int user_id, char *info, int maxlen);

bool    db_disconnect_player(void *handle, unsigned int player_id, unsigned int game_id);
