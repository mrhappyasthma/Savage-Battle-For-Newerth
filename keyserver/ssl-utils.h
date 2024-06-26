#ifndef _COMMON_H_
#define _COMMON_H_

#include "openssl/ssl.h"
#include "auth_common.h"

void    init_ssl(bool server);
SSL     *new_ssl();
bool    free_ssl(SSL *ssl);
void	shutdown_ssl();

void	print_ssl_error(int error);

int set_nonblocking(int socket);

#endif
