#include "message_protocol.h"
#include "commands.h"
#include <time.h>
#include <signal.h>

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
void init_IO_multiplexing(int, mfd *);
void close_serv(int, mfd *);
void accept_clnt_multiplexed(int, mfd *);
timeval init_timeout(int, int);

