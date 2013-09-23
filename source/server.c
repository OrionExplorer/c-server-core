#include <stdio.h>
#include <signal.h>
#include "include/socket_io.h"

void app_terminate( void ) {
	printf( "\rService is being closed..." );
	SOCKET_stop();
	printf( "ok.\n" );
	return;
}

int main( void ) {

	signal( SIGINT, ( sighandler )&app_terminate );
	#ifndef _WIN32
		signal( SIGPIPE, SIG_IGN );
	#endif
	signal( SIGABRT, ( sighandler )&app_terminate );
	signal( SIGTERM, ( sighandler )&app_terminate );

	printf( "c-server-core\n");

	SOCKET_main();

	return 0;
}
