
char *get_timestamp();

bool        server_tcplisten(int *listen_sock, int port);
bool    	server_udplisten(int *udp_sock, int port);
bool    	process_request(int socket, char *buf, int len, char *remote_ip);

bool    tell_gameserver_about_new_patch(char *server, int port);
