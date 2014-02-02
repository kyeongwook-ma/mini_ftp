#include <signal.h>

void 
sig_handler(int signo)
{
    puts("SIGINT RECEIVED");
}



int
main(int argc, char *argv[])
{   
    
	
	/* create server socket with port number */
	create_sock(7777);

	/* register sig handler */
    signal(SIGINT, (void *)sig_handler);

	/* bind address to server */
    if(bind_sock_addr() == -1 )
    {
        logging("connection error");
    }

	/* listen from client waiting queue 5 */
    if(listen_from_clnt(5) ==-1)
    {	
        logging("listen error");
    }

    init_IO_multiplexing();

    run_server();
 
    
    return 0;
}

