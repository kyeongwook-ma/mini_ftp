#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "message_protocol.h"

#define BUF_SIZE 1024
void error_handling(char *);
packet *create_message(char *);

typedef struct st_cmd_type {
	void *(*body_creator)(char *);
	char *cmd_name;
	int msg_type;
}cmd_type;

void *create_save_body(char *);
void *create_save_body(char *);
void *create_ls_body(char *);
void *create_rmdir_body(char *);
void *create_login_body(char *);
void *create_mkdir_body(char *);
void *create_cat_body(char *);
void *create_cd_body(char *);
void *create_logout_body(char *);

cmd_type cmd_list[] = {
	{create_save_body, "save" },
	{create_ls_body, "ls" },
	{create_rmdir_body, "save" },
	{create_login_body, "login"},
	{create_mkdir_body, "mkdir"},
	{create_cat_body, "cat"},
	{create_cd_body, "cd"},
	{create_logout_body, "logout"}
};
