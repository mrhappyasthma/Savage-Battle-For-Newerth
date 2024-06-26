#include "core.h"
#include "auth_common.h"

#define CDKEY_SIZE 20

SSL 	*create_ssl_connection(int socket);
bool 	send_auth_data(SSL *ssl, char *buf, int buflen);
int 	wait_for_response(SSL *ssl, char *cookie, int cookie_length);

static 	SSL *ssl;
static int keyserver_sock;

int		client_connect();

int		connectTime = 0;

extern cvar_t irc_nickname;
extern cvar_t user_id;
extern cvar_t cdkey;
extern cvar_t tcp_timeout;

extern cvar_t server_port;


extern bool authenticating_createaccount;

bool Keyserver_Authenticate(char *server_address, char *username, char *password, char *_cdkey, bool createAccount)
{
	int buflen;
	char buf[MAX_AUTH_PACKET_SIZE];
	char short_cdkey[CDKEY_SIZE+1] = {0};
	char *server_ipaddress = NULL;
	char *tmp;
	bool authenticated = true;
	char sum_char;
	int i, sum = 0, j = 0;

//#ifdef SAVAGE_DEMO
	return true;	//demo doesn't need to bother with authentication
//#endif

	if (strlen(_cdkey) < CDKEY_SIZE)
	{
		Console_Errorf("Invalid CD-Key");
		Cmd_Exec("uiState cdkey");
		authenticated = false;
		return false;
	}
	
	short_cdkey[CDKEY_SIZE] = 0;
	for (i = 0; i < strlen(_cdkey); i++)
	{
		if (_cdkey[i] == 'o' || _cdkey[i] == 'O')
		{
			short_cdkey[j] = '0';
			if (j != CDKEY_SIZE-1)
				sum += short_cdkey[j];
			j++;
		}
		else if ((_cdkey[i] >= 'A' && _cdkey[i] <= 'Z')
			|| (_cdkey[i] >= '0' && _cdkey[i] <= '9'))
		{
			short_cdkey[j] = _cdkey[i];
			if (j != CDKEY_SIZE-1)
				sum += short_cdkey[j];
			j++;
		}
		else if (_cdkey[i] >= 'a' && _cdkey[i] <= 'z')
		{
			short_cdkey[j] = 'A' + _cdkey[i] - 'a';
			if (j != CDKEY_SIZE-1)
				sum += short_cdkey[j];
			j++;
		}
		if (j >= CDKEY_SIZE)
			break;
	}
	sum_char = short_cdkey[CDKEY_SIZE-1];

	if (!sum_char)
	{
		Console_Errorf("Invalid CD-Key");
		Cmd_Exec("uiState cdkey");
		authenticated = false;
		return false;
	}
	
	sum = sum % 16;
	if (sum < 10)
	{
		if (sum_char != '0' + sum)
		{
			Console_Errorf("Invalid CD-Key");
			Cmd_Exec("uiState cdkey");
			authenticated = false;
			return false;
		}
	}
	else 
	{
		if (sum_char != 'A' + sum - 10)
		{
			Console_Errorf("Invalid CD-Key");
			Cmd_Exec("uiState cdkey");
			authenticated = false;
			return false;
		}
	}

	return true;	//demo doesn't need to bother with authentication

	if (server_address)
	{
		server_ipaddress = Tag_Strdup(server_address, MEM_NET);
		if ((tmp = strchr(server_ipaddress, ':')))
		{
			tmp[0] = 0;
			Cvar_SetVarValue(&server_port, atoi(&tmp[1]));
		}
		else
			Cvar_SetVarValue(&server_port, DEFAULT_SERVER_PORT);
	}

	if (!lc.cstate && !authenticating_createaccount)
		return true;		//not trying to connect anywhere, so succeed on local checksum
	
	init_ssl(false);

	keyserver_sock = client_connect();

	if (keyserver_sock <= 0)
	{
		char *coreInfo, *gameInfo, *fw;
		int ping;

		Console_Printf("Couldn't connect to keyserver\n");
		authenticated = true;  //if we can't connect, let them go for now
		
		//let's see if the server is firewalled - if not, don't auth them, it will just fail
		if (server_ipaddress && MasterServer_GetGameInfo(server_ipaddress, server_port.integer, &ping, &coreInfo, &gameInfo))
		{
			fw = ST_GetState(coreInfo, "fw");
			if (strcmp(fw, "1") != 0)
			{
				Console_Errorf("Couldn't contact authentication server at %s:%i", KEYSERVER_IP, KEYSERVER_LISTEN_PORT);
				authenticated = false;
				return 0;
			}
		}
	}
	else
	{
		if (username && password)
		{
			if (createAccount)
			{
				BPrintf(buf, MAX_AUTH_PACKET_SIZE, "CREATEUSER 1\nBUILD %s\nPLATFORM %s\nCDKEY %s\nUSER %s\nPASS %s\n", cl_api.GetBuild(), PLATFORM, short_cdkey, username, password);
			}
			else if (server_ipaddress && server_ipaddress[0])
			{
				BPrintf(buf, MAX_AUTH_PACKET_SIZE, "SERVER %s\nPORT %i\nBUILD %s\nPLATFORM %s\nCDKEY %s\nUSER %s\nPASS %s\n", server_ipaddress, server_port.integer, cl_api.GetBuild(), PLATFORM, short_cdkey, username, password);
			}
			else 
				return 0;
		}
		else if (_cdkey)
		{
			BPrintf(buf, MAX_AUTH_PACKET_SIZE, "BUILD %s\nPLATFORM %s\nCDKEY %s\n", cl_api.GetBuild(), PLATFORM, short_cdkey);
		}
		buflen  = strlen(buf);

		//Console_Printf("client: sending the following authentication data:\n%s\n", buf);
		if (!(ssl = create_ssl_connection(keyserver_sock)))
		{
			Console_Printf("Couldn't get SSL connection\n");
			authenticated = false;
		}
		else
		{
			set_nonblocking(keyserver_sock);

			send_auth_data(ssl, buf, buflen);

			connectTime = System_Milliseconds();
			Console_Printf("SSL connection successful\n");
		}
	}

	if (server_ipaddress)
		Tag_Free(server_ipaddress);
	
	return authenticated;
}

void	Keyserver_Cleanup()
{
	//while (!free_ssl(ssl));

	Console_DPrintf("closing ssl socket\n");
	
#ifdef _WIN32
	closesocket(keyserver_sock);
#else
	close(keyserver_sock); 
#endif
	keyserver_sock = 0;
	
	shutdown_ssl();
	ssl = 0;
}

int	Keyserver_AuthenticateFrame(char *cookie, int cookie_length)
{
	int response;
		
	return 1;

	if (!ssl)
	{
		//we must have failed to connect to the keyserver, just return 1 and let them play
		return 1;
	}

	if (System_Milliseconds() - connectTime > 15000)  //15 second timeout
	{
		Keyserver_Cleanup();
		return 1;
	}

	response = wait_for_response(ssl, cookie, cookie_length);
	if (response == -1)
		return -1;

	Keyserver_Cleanup();
	if (!response)
	{
		return 0;
	}
	return 1;
} 

// process the response we get back from the keyserver
// eventually the keyserver can send us messages, like that we need to upgrade (with a URL to the patch file), etc.
bool	process_keyserver_response(char *buf, int len, char *cookie, int cookie_length)
{
	if (strncmp(buf, AUTH_SUCCESS, strlen(AUTH_SUCCESS)) == 0)
	{
		strncpy(cookie, &buf[strlen(AUTH_SUCCESS)], cookie_length);
		if (atoi(&buf[strlen(AUTH_SUCCESS)+cookie_length+1]))
			Cvar_SetVarValue(&user_id, atoi(&buf[strlen(AUTH_SUCCESS)+cookie_length+1]));
		return true;
	}

	Console_Errorf("Authentication failed: %s", &buf[strlen(AUTH_FAILURE)]);
	if (strstr(&buf[strlen(AUTH_FAILURE)], "Invalid CD KEY"))
		Cmd_Exec("uiState cdkey");
	return false;
}

#define MAX_SSL_CONNECT_ATTEMPTS 160

//set up the SSL handshake and move into encypted mode
SSL *create_ssl_connection(int socket)
{
	SSL *ssl;
	int ret, error, attempt;
	float start = System_Milliseconds();

	ssl = new_ssl();
	if (!ssl)
	{
		Console_Printf("Failed to create the SSL connection structure.\n");
		return NULL;
	}

	SSL_set_fd(ssl, socket);                  	// assign a file descriptor 

	//tell it to read ahead off the network, we won't use select()
	//SSL_set_read_ahead(ssl, true);

	ret = 0;
	attempt = 0;
	while (ret != 1 && attempt < MAX_SSL_CONNECT_ATTEMPTS)
	{
		attempt++;
		ret = SSL_connect(ssl);			 	// accept the SSL connection
		if (ret != 1)
		{
			error = SSL_get_error(ssl, ret);
			if (error == SSL_ERROR_NONE
				|| error == SSL_ERROR_WANT_READ 
				|| error == SSL_ERROR_WANT_WRITE
				|| error == SSL_ERROR_WANT_CONNECT
				|| error == SSL_ERROR_SYSCALL
#ifdef SSL_ERROR_WANT_ACCEPT
				|| error == SSL_ERROR_WANT_ACCEPT)
#else
				)
#endif
			{
#ifdef _WIN32
				Sleep(200);
#else
				usleep(1000);
#endif
				continue;
			}
			/*
			if (error == SSL_ERROR_SYSCALL)
			{
#ifdef _WIN32
				Console_Errorf("SSL Connection failed - %s (error %i)\n", WSAGetLastError(), errno);
#else
				Console_Errorf("SSL Connection failed - %s (error %i)\n", strerror(errno), errno);
#endif
				if (errno == 0)
					continue;
			}
			*/
			print_ssl_error(error);
			Console_Printf("Failed to connect using SSL (error %i) - \n", error);
			while (!free_ssl(ssl));
			return NULL;
		}
	}
	if (attempt >= MAX_SSL_CONNECT_ATTEMPTS)
	{
		Console_Printf("Too many failed attempts!\n");
		while (!free_ssl(ssl));
		return NULL;
	}
	Console_DPrintf("SSL Connection took %fms\n", System_Milliseconds() - start);
	return ssl;
}

//send our authentication string to the keyserver
bool send_auth_data(SSL *ssl, char *buf, int buflen)
{
	if (SSL_write(ssl, buf, buflen) < buflen)
	{
		Console_Printf("Failed to write to the SSL socket.\n");
		return false;
	}
	return true;
}

//wait for a response from the server and then return true if it says we can continue
int wait_for_response(SSL *ssl, char *cookie, int cookie_length)
{
	int len, ret, error;
	char buf[MAX_AUTH_PACKET_SIZE];

	ret = 0;
	error = SSL_ERROR_NONE;
	while (ret == 0 && error != SSL_ERROR_ZERO_RETURN)
	{
		len = SSL_read(ssl,buf,MAX_AUTH_PACKET_SIZE);
		if (len > 0)
		{
			buf[len] = 0;
			ret = 0;
			Console_DPrintf("got %i bytes from the keyserver.\n", len);
			Console_DPrintf("%s\n", buf);
		 	if (process_keyserver_response(buf, len, cookie, cookie_length))
		 	{
				return true;
		 	}
		 	else
		 	{
				return false;
			}
		}
		else if (len == 0)
		{
			return -1;
		}
		else
		{
			ret = len;
			error = SSL_get_error(ssl, ret);
			if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
			{
				Console_Printf("SSL Error: ");
				print_ssl_error(error);
				Console_Printf("\n");
				return false;
			}
			else
				return -1;
		}
	}
	return false;
}

int		client_connect()
{
	int s;
	//char mac[6];
	struct sockaddr_in sockaddr;
	struct hostent *keyserver;
	struct timeval timeo;
	int i;

	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		Console_Printf("Error opening local client socket.\n");
		return -1;
	}

	Console_Printf("connecting to keyserver at %s on port %i\n", KEYSERVER_IP, KEYSERVER_LISTEN_PORT);

	keyserver = gethostbyname(KEYSERVER_IP);
	if (!keyserver)
	{
		Console_Errorf("Unable to resolve %s\n", KEYSERVER_IP);
		return -1;
	}
	
	//set_nonblocking(s);
	
	timeo.tv_sec  = tcp_timeout.value;
 	timeo.tv_usec = 0;
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeo, sizeof (timeo)) < 0)
	{
		Console_Errorf("unable to set socket option SO_RCVTIMEO - %s\n", strerror(errno));
	}

	timeo.tv_sec  = tcp_timeout.value;
	timeo.tv_usec = 0;
	if (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeo, sizeof (timeo)) < 0)
	{
		Console_Errorf("unable to set socket option SO_SNDTIMEO - %s\n", strerror(errno));
	}
	
	sockaddr.sin_family = PF_INET;
	sockaddr.sin_port = htons((unsigned short)KEYSERVER_LISTEN_PORT);

	i = 0;
	while (keyserver->h_addr_list[i])
	{
		memcpy(&sockaddr.sin_addr, keyserver->h_addr_list[i], keyserver->h_length);

		if (connect(s, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) >= 0)
		{
			return s;
		}
		i++;
	}

	Console_Errorf("Failed to connect to Authentication Server!\n");
	return -1;
}

