/*
 * WendzelNNTPd is distributed under the following license:
 *
 * Copyright (c) 2004-2024 Steffen Wendzel <wendzel (at) hs-worms (dot) de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.	If not, see <http://www.gnu.org/licenses/>.
 */

#include "main.h"

extern char *tls_cipher_prio; /* config.y */

static SSL_CTX *context;

#define CHECK(cmd)	\
	return_code = cmd; \
	if (!return_code) { \
		DO_SYSL("TLS not initialized, error in") \
		DO_SYSL(#cmd) \
		fprintf(stderr, "TLS not initialized, %s error\n", #cmd); \
		fprintf(stderr, "%i\n", SSL_get_error((const SSL*) context, return_code)); \
		return FALSE; \
	}

#define CHECK_CTX(cmd)	\
	return_code = cmd; \
	if (return_code != 1) { \
		DO_SYSL("TLS not initialized, error in") \
		DO_SYSL(#cmd) \
		fprintf(stderr, "TLS not initialized, %s error\n", #cmd); \
		ERR_print_errors_fp(stderr); \
		return FALSE; \
	}

int
tls_global_init(sockinfo_t *sockinf)
{
	ERR_load_crypto_strings();
	SSL_load_error_strings();

    int return_code;
	const SSL_METHOD *method;
	method = TLS_server_method();
	context = SSL_CTX_new(method);
	if (!context) {
		DO_SYSL("TLS not initialized, error in SSL_CTX_new()")
		fprintf(stderr, "TLS not initialized, SSL_CTX_new() error\n");
		return FALSE;
	}

    if (sockinf->connectorinfo->ciphers) {
		CHECK_CTX(SSL_CTX_set_cipher_list(context, sockinf->connectorinfo->ciphers));
	}

	DO_SYSL("TLS initialized");
	return TRUE;
}

void
tls_global_close()
{
	SSL_CTX_free(context);

	DO_SYSL("TLS shutdown");
}

int
tls_session_init(SSL **session, int sockfd)
{
	int return_code;

	*session = SSL_new(context);
	if (!*session) {
		DO_SYSL("TLS session not initialized, error in SSL_new()");
		fprintf(stderr, "TLS session not initialized, SSL_new() error\n");
		return FALSE;
	}

	CHECK(SSL_set_fd(*session, sockfd));
	CHECK(SSL_accept(*session));

	DO_SYSL("TLS session init");
	return TRUE;
}

void
tls_session_close(SSL *session)
{
	SSL_shutdown(session);
	SSL_free(session);

	DO_SYSL("TLS session shutdown")
}