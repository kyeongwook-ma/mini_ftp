#ifndef MESSAGE_PROTOCOL
#define MESSAGE_PROTOCOL
    #include "message_protocol.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>

#include <fcntl.h> 
/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>

#define MALLOC(X) (X *)malloc(sizeof(X))
#define MEMSET(X) memset(X, 0, sizeof(X))
#define READ_FROM_FD(COUNT, MAX, FD, DEST) while(COUNT < MAX) COUNT = read(FD, DEST, MAX);
#define MAXLINE 100
#define BUF_SIZE 1024

typedef struct sockaddr_in sock_addr;
sock_addr clnt_addr;

typedef struct st_multiplex_fd {
    fd_set reads, cpy_reads;
    int fd_max, fd_num;
}mfd;

typedef struct timeval timeval;

enum msg_type {LS_REQ = 2, LS_RESP, CAT_REQ, CAT_RESP, SAVE_REQ, SAVE_RESP, 
		RM_REQ, RM_RESP,MKDIR_REQ, MKDIR_RESP, RMDIR_REQ, RMDIR_RESP,
		CD_REQ, CD_RESP, CP_REQ, CP_RESP, LOGIN_REQ, LOGIN_RESP,
		LOGOUT_REQ, LOGOUT_RESP, ERROR = -1};


typedef struct st_account {
    char id[MAXLINE];
    char pw[MAXLINE];
}acc;

typedef struct st_sock {
	int sock_fd;
	sock_addr sock_addr;
}sock;


packet create_message(char *, int);
char *process_message(packet *);

bool bind_sock_addr();
bool listen_from_clnt(int);
void create_sock(int);
void init_IO_multiplexing();
void close_serv();
void accept_clnt_multiplexed();
timeval init_timeout(int, int);
void run_server();


void sig_handler(int signo);
void logging(char *);

typedef struct st_cmd {
    int type;
    void *(*exec)(void *);
}cmd;

void *cmd_cp(void *);
void *cmd_cat(void *);
void *cmd_ls(void *);
void *cmd_save(void *);
void *cmd_cd(void *);
void *cmd_logout(void *);
void *cmd_login(void *);
void *cmd_rmdir(void *);
void *cmd_rm(void *);
void *cmd_mkdir(void *);

cmd CMD_LIST[] = {
    {LS_REQ, cmd_ls},
    {CAT_REQ, cmd_cat},
    {SAVE_REQ, cmd_save},
    {RM_REQ, cmd_rm},
    {MKDIR_REQ, cmd_mkdir},
    {RMDIR_REQ, cmd_rmdir},
    {CD_REQ, cmd_cd},
    {CP_REQ, cmd_cp},
    {LOGIN_REQ, cmd_login},
    {LOGOUT_REQ, cmd_logout},
    {ERROR, NULL }
};
