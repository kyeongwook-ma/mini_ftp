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

packet *create_message(char *command_line)
{
	packet *msg;
	msg_header header;	
	msg_body body;

	int result = 0;
	int packet_size = 0;
	int msg_type = 0;
	int body_size = 0;	

	memset((void *)&header, 0x00, sizeof(msg_header));
	memset((void *)&body, 0x00, sizeof(msg_body));


	// 메세지 타입에 따라 바디 생성
	if(strstr(command_line, "ls"))
	{
		msg_type = LS_REQ;
	}
	
	else if(strstr(command_line, "save"))
	{
		char *filename = command_line;
		char buf[BUF_SIZE] = {0,};
		char *str;

		msg_type = SAVE_REQ;
	
		filename = strtok(filename, " ");
		filename = strtok(NULL, " ");

		fgets(buf,BUF_SIZE, stdin);
		buf[strlen(buf) - 1] = 0;
		
	
		strcat(filename, " ");
		strcat(filename, buf);

		puts(filename);

		strcpy(body.data, filename);

	}
	
	else if(strstr(command_line, "login"))
	{
		int line_size = 10;
	
		char id[line_size];
		char pw[line_size];

		msg_type = LOGIN_REQ;
	
		puts("\nID를 입력하시오 : ");
		fgets(id,line_size,stdin);

		puts("\nPW를 입력하시오 : ");
		fgets(pw,line_size,stdin);

		strcat(id," ");
		strcat(id, pw);
		strcpy(body.data, id);

	}

	else if(strstr(command_line, "mkdir"))
	{
		char *foldername = command_line;

		msg_type = MKDIR_REQ;

		foldername = strtok(foldername, " ");
		foldername = strtok(NULL, " ");

		strcpy(body.data, foldername);
	}

	else if(strstr(command_line, "rmdir"))
	{
		char *foldername = command_line;

		
		msg_type = RMDIR_REQ;

		foldername = strtok(foldername, " ");
		foldername = strtok(NULL, " ");

		puts(foldername);

		strcpy(body.data, foldername);

	}

	else if(strstr(command_line, "cat"))
	{
		char *filename = command_line;

		msg_type = CAT_REQ;

		filename = strtok(filename, " ");
		filename = strtok(NULL, " ");
		
		strcpy(body.data, filename);
	}

	else if(strstr(command_line, "rm"))
	{
		char *filename = command_line;
		msg_type = RM_REQ;

		filename = strtok(filename, " ");
		filename = strtok(NULL, " ");
		
		strcpy(body.data, filename); 
	}

	else if(strstr(command_line, "cd"))
	{
		char *foldername = command_line;

		msg_type = CD_REQ;
	
		if(strcmp(command_line, "cd") == 0)
		{
			foldername = NULL;
		}
		else	
		{
			foldername = strtok(foldername, " ");
			foldername = strtok(NULL, " ");
		}

		strcpy(body.data, foldername);
	}
	
	
	else if(strstr(command_line, "cp"))
	{
		char *filename = command_line;		

		msg_type = CP_REQ;
	
		filename = strtok(filename, " ");
		filename = strtok(NULL, "");

		strcpy(body.data, filename);	
		
	}

	else if(strstr(command_line, "logout"))
	{
		msg_type = LOGOUT_REQ;
		strcpy(body.data, "bye");	
	}

	body_size = sizeof(body);
		
	packet_size = body_size + sizeof(msg_header);
	

	// 헤더에 정보 저장
	header.packet_size = htons(packet_size);
	header.msg_type = htons(msg_type);
	header.body_size = htons(body_size);

	msg = (packet *)malloc(packet_size);

	// 메세지 저장
	msg->header = header;
	msg->body = body;


	return msg;

}

