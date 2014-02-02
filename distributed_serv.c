#include "server_func.h"

int
main(int argc, char *argv[])
{
    int serv_sock;
    sock_addr serv_addr;
    packet recv_msg;
    msg_header recv_header;
    packet *send_msg;


    mfd fd;    
	
	/* create server socket with port number */
	serv_sock = create_serv_sock(&serv_addr, 7777);

	/* register sig handler */
    signal(SIGINT, (void *)sig_handler);

	/* bind address to server */
    if(bind_serv_addr(serv_sock, &serv_addr) == -1 )
    {
        logging("connection error");
    }

	/* listen from client waiting queue 5 */
    if(listen_from_clnt(serv_sock, 5) ==-1)
    {	
        logging("listen error");
    }

    init_IO_multiplexing(serv_sock, &fd);

    while(1)
    {
        fd.cpy_reads = fd.reads;

		/* set timeout */
        timeval timeout = init_timeout(5, 5000);

		/* IO multiplexing through select() */
        if((fd.fd_num = select(fd.fd_max + 1, &(fd.cpy_reads), 0, 0, &timeout)) == -1)
            break;

		/* no connection */
        if(fd.fd_num == 0)
            continue;

		int fd_idx;


        for(fd_idx = 0; fd_idx < fd.fd_max + 1; ++fd_idx) {
            if(FD_ISSET(fd_idx, &(fd.cpy_reads))) {
				/* server socket */
                if(fd_idx == serv_sock) {
					/* accept client */
                    accept_clnt_multiplexed(serv_sock, &fd);

                }

				/* receive from client's request */
                else {

					packet recv_msg, send_msg;
				
					receive_from_clnt(&recv_msg);

                  
					/* response to client's request */
					char *result = 0;

                    int msg_type = ntohs(recv_msg.header.msg_type);

                    /* process message */
					result = process_message(&recv_msg);

                    send_msg = create_message(result, msg_type);

					response_to_clnt(&send_msg);
				}
			}
		}
    }
    
    close(serv_sock);

    return 0;
}



