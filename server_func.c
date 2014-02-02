#include "server_func.h"

bool
bind_serv_addr(int serv_sock, sock_addr *serv_addr)
{
    return bind(serv_sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr));
}

bool
listen_from_clnt(int serv_sock, int capacity)
{
    return listen(serv_sock, capacity);
}

/*
 * set address to socket
 */
int 
create_serv_sock(sock_addr *serv_addr, int port)
{
    int serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(serv_addr, 0, sizeof(serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr->sin_port = htons(port);
   
    return serv_sock;
}

/*
 * init fd sets
 */
void
init_IO_multiplexing(int serv_sock, mfd *fd)
{
    FD_ZERO(&(fd->reads));
    FD_SET(serv_sock, &(fd->reads));
    fd->fd_max = serv_sock;
}

/*
 * half close server
 */
void 
close_serv(int serv_sock, mfd *fd)
{
    FD_CLR(serv_sock, &(fd->reads));
    shutdown(serv_sock, SHUT_WR);
    logging("closed");
}

/*
 * set timeout
 */
timeval
init_timeout(int sec, int usec)
{
    timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    return timeout;
}

/*
 * accept client, register to fd set
 */
void
accept_clnt_multiplexed(int serv_sock, mfd *fd)
{
    int clnt_sock;
    sock_addr clnt_addr;
    int clnt_addr_size = sizeof(clnt_addr);
 
 	/* accept client */
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, clnt_addr_size);

	/* register client to observed socket */
    FD_SET(clnt_sock, &(fd->reads));
 
    if(fd->fd_max < clnt_sock)
        fd->fd_max = clnt_sock;

}

/*
 * process client's request
 */
void
process_request(int clnt_sock)
{
    int str_len = 0;
    int header_size = sizeof(msg_header);
    packet recv_msg, send_msg;
    msg_header *header;
    
    while(str_len < header_size) {
        str_len = read(clnt_sock, (void *)header, header_size);
    }

}

/*
 * receive message from client 
 */
void
receive_from_clnt(packet *recv_msg, int fd)
{
	int str_len = 0;
    int header_size = sizeof(msg_header);

	/* receive message header from client */
	READ_FROM_FD(str_len, header_size, fd, (void *)&(recv_msg->header));

	int body_size = ntohs(recv_msg->header.body_size);

	// ë°ëí¬ê¸° ë§í¼ ëì  í ë¹
	str_len = 0;

	READ_FROM_FD(str_len, body_size, fd, (void *)&(recv_msg->body));

}

void
response_to_clnt(packet *send_msg, int fd)
{
	int write_count = 0;
	int header_size = sizeof(send_msg->header);						
	int body_size = sizeof(send_msg->body);

	while(write_count < header_size) {
		write_count = write(fd, (void *)&send_msg->header, header_size);
	}

	write_count = 0;

	while(write_count < body_size) {
		write_count = write(fd, (void *)&send_msg->body.data, body_size);
	}
	  
	 

}


/*
 * create a response message
 */
char *
process_message(packet *msg)
{
    int msg_type = htons(msg->header.msg_type);
    int i = 0;
    char buf[BUF_SIZE] = {0,};
    char *result;

    strcpy(buf, msg->body.data);

    for( i = 0; sizeof(CMD_LIST) / sizeof(cmd); ++i) {

        cmd msg_cmd = CMD_LIST[i];

        if(msg_cmd.type == msg_type) {
            result = msg_cmd.exec(buf);
            return result;
        }

    }
}

packet
create_message(char *data, int req_type)
{
    packet msg;
    msg_header header;
    msg_body body;

    int msg_type = 0;
    int body_size = 0;

    switch(req_type)
    {
        case LOGIN_REQ :
            msg_type = LOGIN_RESP;
            break;

        case LS_REQ :
            msg_type = LS_RESP;
            break;

        case SAVE_REQ :
            msg_type = SAVE_RESP;
            break;

        case RM_REQ :
            msg_type = RM_RESP;
            break;

        case CP_REQ :
            msg_type = CP_RESP;
            break;

        case CAT_REQ : 
            msg_type = CAT_RESP;
            break;

        case RMDIR_REQ :
            msg_type = RMDIR_RESP;
            break;

        case CD_REQ :
            msg_type = CD_RESP;
            break;

        case MKDIR_REQ :
            msg_type = MKDIR_RESP;
            break;

        default :
            break;
    }

    if(data != NULL)
    {
        strcpy(body.data, data);	
    }

    body_size = sizeof(body);


    header.msg_type = htons(msg_type);
    header.body_size = htons(body_size);

    msg.header = header;
    msg.body = body;

    return msg;
}

void 
sig_handler(int signo)
{
    puts("SIGINT RECEIVED");
}

void
logging(char *msg)
{
    char *clntip = inet_ntoa(clnt_addr.sin_addr);
    FILE *fp;
    time_t timer;

    time(&timer);

    fp = fopen("logfile.txt","a");

    fprintf(fp, "%s| %s ", clntip, msg);
    fprintf(fp, "%s", ctime(&timer));

    fclose(fp);
}
