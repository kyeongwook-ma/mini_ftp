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

static cmd_type cmd_list[] = {
	{create_save_body, "save" },
	{create_ls_body, "ls" },
	{create_rmdir_body, "save" },
	{create_login_body, "login"},
	{create_mkdir_body, "mkdir"},
	{create_cat_body, "cat"},
	{create_cd_body, "cd"},
	{create_logout_body, "logout"}
};


int main(int argc, char *argv[])
{	
	int sock;
	int str_len;
	struct sockaddr_in serv_adr;
	packet *send_msg;
	packet recv_msg;

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");
	
	while(1) 
	{
		char message[BUF_SIZE] = {0,};
		char buf[BUF_SIZE] = {0, };
		int write_count = 0;
		int packet_size = 0;
		int header_size = 0;
		int body_size = 0;

		rewind(stdin);
		memset((void *)message, 0x00, BUF_SIZE);
	
		// 사용자로부터 입력 받음
		fputs("Input Command :", stdout);

		fgets(message, BUF_SIZE, stdin);
		message[strlen(message) - 1] = '\0';
	
		
		// 메세지 처리
		send_msg = (packet *)create_message(message);
		
		header_size = sizeof(send_msg->header);

		// 헤더 사이즈 만큼
		while(write_count < header_size)
		{
			// 서버로 쓰기
			write_count = write(sock, (void *)&send_msg->header, sizeof(send_msg->header));
		}

		write_count = 0;

		body_size = sizeof(send_msg->body);

		// req 바디를 씀
		while(write_count < body_size)
		{
			write_count = write(sock, (void *)&send_msg->body, body_size);
		}


		write_count = 0;	

		int logout = ntohs(send_msg->header.msg_type);
	
		if(logout == LOGOUT_REQ)
			break;

		int read_count = 0;
	
		// 서버의 resp 메세지 헤더를 받음		
		while(read_count < header_size)
		{
			read_count = read(sock, (void *)&recv_msg.header, sizeof(recv_msg.header));
		}

		int resp_msg_type = ntohs(recv_msg.header.msg_type);

		
		// resp 바디의 크기를 구함
		body_size = ntohs(recv_msg.header.body_size);

		read_count = 0;

		// 바디 크기 만큼 읽어들임
		while(read_count < body_size)
		{
			read_count = read(sock, (void *)&buf, body_size);
		}
		

		strcat(buf, "\n");
		puts(buf);

	}
	
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void
create_body(msg_body *body, char *command_line)
{
	int i = 0;

	for(i = 0; i < sizeof(cmd_list) / sizeof(cmd_type); ++i) {
		if(strstr(command_line, cmd_list[i].cmd_name)) {
			strcpy(body->data, cmd_list[i].body_creator(command_line));			
		}
	}
}

void *
create_save_body(char *command_line)
{
	char *filename = command_line;
	char buf[BUF_SIZE] = {0,};
	char *str;


	filename = strtok(filename, " ");
	filename = strtok(NULL, " ");

	fgets(buf,BUF_SIZE, stdin);
	buf[strlen(buf) - 1] = 0;
			
	strcat(filename, " ");
	strcat(filename, buf);

	puts(filename);
	
	return filename;
}

void *
create_ls_body(char *command_line)
{

}

void *
create_login_body(char *command_line)
{
	int line_size = 10;
	
	char *id;
	char *pw;
	
	puts("\nID를 입력하시오 : ");
	fgets(id,line_size,stdin);

	puts("\nPW를 입력하시오 : ");
	fgets(pw,line_size,stdin);

	strcat(id," ");
	strcat(id, pw);
	
	return id;
}

void *
create_mkdir_body(char *command_line)
{
	char *foldername = command_line;


	foldername = strtok(foldername, " ");
	foldername = strtok(NULL, " ");

	return foldername;
}

void *
create_rmdir_body(char *command_line)
{
	char *foldername = command_line;

	foldername = strtok(foldername, " ");
	foldername = strtok(NULL, " ");

	puts(foldername);

	return foldername;

}

void *
create_cat_body(char *command_line)
{
	char *filename = command_line;

	filename = strtok(filename, " ");
	filename = strtok(NULL, " ");
		
	return filename;
}

void *
create_rm_body(char *command_line)
{	
	char *filename = command_line;

	filename = strtok(filename, " ");
	filename = strtok(NULL, " ");
		
	return filename; 
}

void *
create_cd_body(char *command_line)
{	
	char *foldername = command_line;


	
	if(strcmp(command_line, "cd") == 0) {
		foldername = NULL;
	}
	
	else {
		foldername = strtok(foldername, " ");
		foldername = strtok(NULL, " ");
	}

	return foldername;
}
	

void *
create_cp_body(char *command_line)
{
	char *filename = command_line;		

	
	filename = strtok(filename, " ");
	filename = strtok(NULL, "");

	return filename;			
}

void *
create_logout_body(char *command_line)
{	
	return "bye";	
}

void
create_header(msg_header *header, int msg_type, int body_size)
{
	header->packet_size = htons(sizeof(msg_header) + body_size);
	header->msg_type = htons(msg_type);
	header->body_size = htons(body_size);
}

packet *
create_message(char *command_line)
{
	packet *msg;
	msg_header header;	
	msg_body body;

	int result = 0;
	int msg_type = 0;
	int body_size = 0;	

	create_body(&body ,command_line);
	
	body_size = sizeof(body);
			
	// 헤더에 정보 저장
	create_header(&header, msg_type, body_size);

	msg = (packet *)malloc(sizeof(header) + sizeof(body));

	// 메세지 저장
	msg->header = header;
	msg->body = body;

	return msg;

}

