#ifndef SHARED_H
#define SHARED_H

#include "portable.h"
#include <stdio.h>
#include <time.h>

#define FD_SETSIZE	1024

#ifdef _WIN32
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0501
	#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#ifdef _MSC_VER
#pragma comment( lib, "WS2_32.lib" )
#endif

#ifndef _MSC_VER
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifndef _WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/select.h>
#endif

#define MAX_BUFFER							65535
#define MAX_BUFFER_CHAR						65535*sizeof( char )
#define UPLOAD_BUFFER						16384
#define UPLOAD_BUFFER_CHAR					16384*sizeof( char )
#define LOG_BUFFER							128
#define LOG_BUFFER_CHAR						128*sizeof( char )
#define LARGE_BUFF_SIZE						8192
#define LARGE_BUFF_SIZE_CHAR				8192*sizeof( char )
#define BIG_BUFF_SIZE						2048
#define BIG_BUFF_SIZE_CHAR					2048*sizeof( char )
#define MEDIUM_BUFF_SIZE					1024
#define MEDIUM_BUFF_SIZE_CHAR				1024*sizeof( char )
#define STD_BUFF_SIZE						256
#define STD_BUFF_SIZE_CHAR					256*sizeof( char )
#define TIME_BUFF_SIZE						30
#define TIME_BUFF_SIZE_CHAR					30*sizeof( char )
#define SMALL_BUFF_SIZE						32
#define SMALL_BUFF_SIZE_CHAR				32*sizeof( char )
#define TINY_BUFF_SIZE						16
#define TINY_BUFF_SIZE_CHAR					16*sizeof( char )
#define PROTO_BUFF_SIZE						10
#define PROTO_BUFF_SIZE_CHAR				10*sizeof( char )
#define MICRO_BUFF_SIZE						8
#define MICRO_BUFF_SIZE_CHAR				8*sizeof( char )
#define EXT_LEN								8
#define EXT_LEN_CHAR						8*sizeof( char )

#define MAX_PATH_LENGTH						1024
#define MAX_PATH_LENGTH_CHAR				1024*sizeof( char )
#define MAX_CLIENTS							FD_SETSIZE
#define DEFAULT_PORT						1212

typedef struct {
	struct sockaddr_in			address;
#ifdef _WIN32
	SOCKET						socket;
#else
	int							socket;
#endif
	fd_set						socket_data;
#ifdef _WIN32
	int							data_length;
#else
	socklen_t					data_length;
#endif
	int							socket_descriptor;
	char*						content[ MAX_BUFFER ];
	short						keep_alive;
} COMMUNICATION_SESSION;

#ifdef _WIN32
	extern WSADATA				wsk;
	extern SOCKET				socket_server;
#else
	extern int					socket_server;
#endif
extern int						addr_size;
extern int						active_port;
extern struct sockaddr_in		server_address;
extern int						ip_proto_ver;
extern COMMUNICATION_SESSION	communication_session_;
extern fd_set					master;

typedef struct {
	int							socket_descriptor;
	short						connected;
} CONNECTED_CLIENT;

CONNECTED_CLIENT				connected_clients[ MAX_CLIENTS ];

#endif
