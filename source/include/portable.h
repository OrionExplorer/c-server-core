#ifndef PORTABLE_H
#define PORTABLE_H

/* #undef _WIN32 */

#ifdef _WIN32
	#include <direct.h>
	#define SLASH		"\\"
	#define C_SLASH		'\\'
	#define READ_BINARY	"rb"
	#define popen		_popen
	#define pclose		_pclose
	#define putenv		_putenv
	#define chdir		_chdir
	#define mkdir( a,b )	mkdir( a )
	#define S_IREAD		_S_IREAD
	#define fcntl		ioctlsocket
	#define close		closesocket
	#define SHUT_RDWR	SD_BOTH
	#define F_SETFL		FIONBIO
	#define S_IXOTH		S_IEXEC
	#define sighandler	__p_sig_fn_t
	#define MSG_NOSIGNAL	0
	#define sleep		Sleep
#else
	#define WSAGetLastError() errno
	#define SLASH		"/"
	#define C_SLASH		'/'
	#define SOCKET_ERROR ( -1 )
	#define EXIT_FAILURE ( 1 )
	#define READ_BINARY	"re"
	#define sighandler	__sighandler_t
	#define Sleep(x)	usleep(x*1000)
#endif

#ifndef NI_MAXHOST
# define NI_MAXHOST			1025
#endif
#ifndef NI_MAXSERV
# define NI_MAXSERV			32
#endif
#ifndef NI_NUMERICHOST
# define NI_NUMERICHOST		1
#endif
#ifndef NI_NUMERICSERV
# define NI_NUMERICSERV		2
#endif
#ifndef NI_NOFQDN
# define NI_NOFQDN			4
#endif
#ifndef NI_NAMEREQD
# define NI_NAMEREQD		8
#endif
#ifndef NI_DGRAM
# define NI_DGRAM			16
#endif

#endif
