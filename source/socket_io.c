#include "include/socket_io.h"
#include "include/shared.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>


#ifdef _WIN32
	WSADATA				wsk;
	SOCKET				socket_server;
#else
	int					socket_server;
#endif

int						addr_size;
int						active_port;
struct sockaddr_in		server_address;
COMMUNICATION_SESSION	communication_session_;
int						i_sac;
fd_set					master;
fd_set					read_fds;
int						fdmax;
int						newfd;
struct hostent			*host;
struct in_addr			addr;

int						http_conn_count = 0;

static void		SOCKET_initialization( void );
static void		SOCKET_prepare( void );
static void		SOCKET_process( int socket_fd );
void			SOCKET_stop( void );

static void SOCKET_initialization( void ) {
	printf("Initializing...");

	#ifdef _WIN32
		if ( WSAStartup( MAKEWORD( 2, 2 ), &wsk ) != 0 ) {
			printf( "error creating Winsock.\n" );
			system( "pause" );
			exit( EXIT_FAILURE );
		}
	#endif

	socket_server = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( socket_server == SOCKET_ERROR ) {
		printf( "error creating socket.\n" );
		SOCKET_stop();
		exit( EXIT_FAILURE );
	}

	active_port = DEFAULT_PORT;

	memset( &server_address, 0, sizeof( server_address ) );
	server_address.sin_addr.s_addr = htonl( INADDR_ANY );
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons( ( unsigned short )active_port );
}

static void SOCKET_prepare( void ) {
	unsigned long b = 0;
	int i = 1;
	int wsa_result = 0;
	struct timeval tv = {0, 0};

	tv.tv_sec = 0;
	tv.tv_usec = 20000;

	FD_ZERO( &master );
	FD_ZERO( &read_fds );

#ifndef _WIN32
	setuid( 0 );
	setgid( 0 );
#endif

	if( setsockopt( socket_server, SOL_SOCKET, SO_REUSEADDR, &i, sizeof( int ) ) == SOCKET_ERROR ) {
		wsa_result = WSAGetLastError();
		printf( "setsockopt( SO_REUSEADDR ) error: %d.\n", wsa_result );
	}

	if ( bind( socket_server, ( struct sockaddr* )&server_address, sizeof( server_address ) ) == SOCKET_ERROR ) {
		wsa_result = WSAGetLastError();
		printf( "bind error: %d.\n", wsa_result );
		SOCKET_stop();
		exit( EXIT_FAILURE );
	}

	if( setsockopt( socket_server, SOL_SOCKET, SO_RCVTIMEO, ( char* )&tv, sizeof( struct timeval ) ) == SOCKET_ERROR ) {
		wsa_result = WSAGetLastError();
		printf( "setsockopt( SO_RCVTIMEO ) error: %d.\n", wsa_result );
	}

	if( setsockopt( socket_server, SOL_SOCKET, SO_SNDTIMEO, ( char* )&tv, sizeof( struct timeval ) ) == SOCKET_ERROR ) {
		wsa_result = WSAGetLastError();
		printf( "setsockopt( SO_SNDTIMEO ) error: %d.\n", wsa_result );
	}

	if( setsockopt( socket_server, IPPROTO_TCP, TCP_NODELAY, ( char * )&i, sizeof( i ) ) == SOCKET_ERROR ) {
		wsa_result = WSAGetLastError();
		printf( "setsockopt( TCP_NODELAY ) error: %d.\n", wsa_result );
	}

	if( fcntl( socket_server, F_SETFL, &b ) == SOCKET_ERROR ) {
		wsa_result = WSAGetLastError();
		printf( "ioctlsocket error: %d.\n", wsa_result );
		SOCKET_stop();
		exit( EXIT_FAILURE );
	}

	if( listen( socket_server, MAX_CLIENTS ) == SOCKET_ERROR ) {
		wsa_result = WSAGetLastError();
		printf( "listen error: %d.\n", wsa_result );
		SOCKET_stop();
		exit( EXIT_FAILURE );
	}

	printf( "ok.\n" );
}

void SOCKET_run( void ) {
	register int i = 0;
	struct timeval tv;
	pthread_t sthread;

	FD_SET( socket_server, &master );

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	fdmax = socket_server;

	for( ;"elvis presley lives"; ) {
		read_fds = master;
		if( select( fdmax+1, &read_fds, NULL, NULL, &tv ) == -1 ) {
			SOCKET_stop();
			exit( EXIT_FAILURE );
		}

		i = fdmax+1;
		while( --i ) {
			if( FD_ISSET( i, &read_fds ) ) {
				if( i == socket_server ) {
					SOCKET_modify_clients_count( 1 );

					communication_session_.data_length = sizeof( struct sockaddr );
					newfd = accept( socket_server, ( struct sockaddr* )&communication_session_.address, &communication_session_.data_length );
					communication_session_.socket_descriptor = newfd;

					if( newfd == -1 ) {
						printf( "Socket closed.\n" );
					} else {
						SOCKET_register_client( newfd );
						FD_SET( newfd, &master );
						if( newfd > fdmax ) {
							fdmax = newfd;
						}
					}
				} else {
					SOCKET_process( i );
				}
			}
		}
	}
}

static void SOCKET_process( int socket_fd ) {
	CONNECTED_CLIENT *client = SOCKET_find_client( socket_fd );
	extern int errno;

	errno = 0;
	communication_session_.socket_descriptor = socket_fd;
	memset( communication_session_.content, '\0', MAX_BUFFER );

	communication_session_.data_length = recv( ( int )socket_fd, communication_session_.content, MAX_BUFFER, 0 );

	if( errno > 1) {
		SOCKET_unregister_client( socket_fd );
		SOCKET_close( socket_fd );
	} else {
		if ( communication_session_.data_length <= 0 ) {
			SOCKET_unregister_client( socket_fd );
			SOCKET_close( socket_fd );
		} else {
			printf( "Received packet with size %ld.\n", communication_session_.data_length );
		}
	}
}

void SOCKET_modify_clients_count( int mod ) {
	if( mod > 0 ) {
		http_conn_count++;
	} else {
		if( (http_conn_count - mod) >= 0 ) {
			http_conn_count--;
		}
	}
}

void SOCKET_close( int socket_descriptor ) {
	FD_CLR( socket_descriptor, &master );
	shutdown( socket_descriptor, SHUT_RDWR );
	close( socket_descriptor );
	SOCKET_modify_clients_count( -1 );
}

void SOCKET_stop( void ) {
	/*shutdown( socket_server, SHUT_RDWR );*/
	/*close( communication_session_.socket );*/
	close( communication_session_.socket );
	close( socket_server );

	#ifdef _WIN32
		WSACleanup();
	#endif
}

void SOCKET_release( COMMUNICATION_SESSION *communication_session ) {
	communication_session->socket_descriptor = -1;
	communication_session->data_length = -1;
	communication_session->keep_alive = -1;
}

void SOCKET_disconnect_client( COMMUNICATION_SESSION *communication_session ) {
	if( communication_session->socket_descriptor != SOCKET_ERROR ) {
		SOCKET_close( communication_session->socket_descriptor );
	} else {
		SOCKET_release( communication_session );
	}
}

void SOCKET_send( COMMUNICATION_SESSION *communication_session, CONNECTED_CLIENT *client, const char *data, unsigned int data_size ) {
	int _data_size = data_size;
	char *data_to_send = ( char * )calloc( MAX_BUFFER, sizeof( char ) );

	if( _data_size == -1 ) {
		_data_size = strlen( data );
	}

	strncpy( data_to_send, data, MAX_BUFFER );

	if( ( communication_session->data_length = send( client->socket_descriptor, data_to_send, _data_size, 0 ) ) <= 0 ) {
		SOCKET_disconnect_client( communication_session );
	}

	free( data_to_send );
	data_to_send = NULL;
}

char* SOCKET_get_remote_ip( COMMUNICATION_SESSION *communication_session ) {
	static char ip_addr[ TINY_BUFF_SIZE ];
	memset( ip_addr, '\0', TINY_BUFF_SIZE );
	getnameinfo( ( struct sockaddr * )&communication_session->address, sizeof( communication_session->address ), ip_addr, sizeof( ip_addr ), NULL, 0, NI_NUMERICHOST );
	return ( ( char* )&ip_addr );
}

void SOCKET_main( void ) {
	( void )SOCKET_initialization();
	( void )SOCKET_prepare();
	( void )SOCKET_run();
	( void )SOCKET_stop();
}

void SOCKET_register_client( int socket_descriptor ) {
	int i;

	for( i = 0; i < MAX_CLIENTS; i++ ){
		if( connected_clients[ i ].socket_descriptor == 0 ) {
			connected_clients[ i ].socket_descriptor = socket_descriptor;
			connected_clients[ i ].connected = 1;
			printf("Client connected with descriptor %d.\n", socket_descriptor );
			return;
		}
	}
}

void SOCKET_unregister_client( int socket_descriptor ) {
	int i;

	for( i = 0; i < MAX_CLIENTS; i++ ){
		if( connected_clients[ i ].socket_descriptor == socket_descriptor ) {
			connected_clients[ i ].socket_descriptor = 0;
			connected_clients[ i ].connected = 0;
			printf("Client disconnected: %d.\n", socket_descriptor );
			break;
		}
	}
}

CONNECTED_CLIENT* SOCKET_find_client( int socket_descriptor ) {
	int i;

	for( i = 0; i < MAX_CLIENTS; i++ ){
		if( connected_clients[ i ].socket_descriptor == socket_descriptor ) {
			return &connected_clients[ i ];
		}
	}
}
