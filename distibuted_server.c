#include "message_protocol.h"
#include "commands.h"
#include <time.h>
#include <signal.h>

#define MALLOC(X) (X *)malloc(sizeof(X))
#define MEMSET(X) memset(X, 0, sizeof(X))
#define READ_FROM_FD(COUNT, MAX, FD, DEST) while(COUNT < MAX) COUNT = read(FD, DEST, MAX);

char *process_message(packet *);
void logging(char *);
packet *create_message(char *, int);
void sig_handler(int signo);


typedef struct sockaddr_in sock_addr;
sock_addr clnt_addr;

typedef struct st_multiplex_fd {
    fd_set reads, cpy_reads;
    int fd_max, fd_num;
}mfd;

typedef struct timeval timeval;

bool bind_serv_addr(int, sock_addr *);
bool listen_from_clnt(int, int);
int create_serv_sock(sock_addr *, int);
int init_IO_multiplexing(int, mfd *);
int close_serv(int, mfd *);
void accept_clnt_multiplexed(int, mfd *);
timeval init_timeout(int, int);

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
int
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
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

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

int
main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    int option = 1;
    sock_addr serv_addr;
    packet recv_msg;
    msg_header recv_header;
    packet *send_msg;


    socklen_t addr_size;
    int i;

    mfd fd;
    
    char buf[BUF_SIZE] = {0, };
	
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

        for(i = 0; i < fd.fd_max + 1; ++i) {
            if(FD_ISSET(i, &(fd.cpy_reads))) {
				/* server socket */
                if(i == serv_sock) {
					/* accept client */
                    accept_clnt_multiplexed(serv_sock, &fd);

                }

				/* process client's request */
                else {

                    int str_len = 0;
                    int header_size = sizeof(msg_header);

                    // 헤더를 받�
                    READ_FROM_FD(str_len, header_size, i, (void *)&recv_header);
					
					while(str_len < header_size)
                    {
                        str_len = read(i, (void *)&recv_header, header_size);
                    }

                    int packet_size = ntohs(recv_header.packet_size);
                    int msg_type = ntohs(recv_header.msg_type);
                    int body_size = ntohs(recv_header.body_size);

                    // 바디크기 만큼 동적 할당
                    str_len = 0;

                    while(str_len < body_size) {
                        str_len = read(i, (void *)&recv_msg.body, body_size);
                    }

                    recv_msg.header = recv_header;

                    /* half close socket */
                    if(str_len == 0 || msg_type == LOGOUT_REQ) {
                        close_serv(i, &fd);
                    }


                    else  {		
                        
                        char *result = 0;

                        int msg = ntohs(recv_msg.header.msg_type);

                        /* process message */
                        result = process_message(&recv_msg);

                        send_msg = create_message(result, msg);

                        int write_count = 0;
                        int header_size = sizeof(send_msg->header);						
                        int body_size = sizeof(send_msg->body);

                        while(write_count < header_size)
                        {
                            write_count = write(i, (void *)&send_msg->header, header_size);
                        }

                        write_count = 0;

                        while(write_count < body_size)
                        {
                            write_count = write(i, (void *)&send_msg->body.data, body_size);
                        }


                    }
                }
            }
        }
    }
    close(serv_sock);

    return 0;
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

packet *
create_message(char *data, int req_type)
{
    packet *msg;
    msg_header header;
    msg_body body;

    int result = 0;
    int msg_type = 0;
    int packet_size = 0;
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

    packet_size = body_size + sizeof(header);

    header.packet_size = htons(packet_size);
    header.msg_type = htons(msg_type);
    header.body_size = htons(body_size);

    msg = (packet *)malloc(packet_size);

    msg->header = header;
    msg->body = body;

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
