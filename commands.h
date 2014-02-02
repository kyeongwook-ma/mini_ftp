#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>

#define MAXLINE 100
#define BUF_SIZE 1024

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

enum msg_type {LS_REQ = 2, LS_RESP, CAT_REQ, CAT_RESP, SAVE_REQ, SAVE_RESP, 
		RM_REQ, RM_RESP,MKDIR_REQ, MKDIR_RESP, RMDIR_REQ, RMDIR_RESP,
		CD_REQ, CD_RESP, CP_REQ, CP_RESP, LOGIN_REQ, LOGIN_RESP,
		LOGOUT_REQ, LOGOUT_RESP, ERROR = -1};

typedef struct st_account {
    char id[MAXLINE];
    char pw[MAXLINE];
}acc;

typedef struct st_cmd {
    int type;
    void *(*exec)(void *);
}cmd;

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



