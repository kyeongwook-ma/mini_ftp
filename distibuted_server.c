#include "message_protocol.h"
#include "commands.h"
#include <time.h>
#include <signal.h>

char *process_message(packet *);
void logging(char *);
packet *create_message(char *, int);
void sig_handler(int signo);

struct sockaddr_in clnt_addr;

int
main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    int option = 1;
    struct sockaddr_in serv_addr;
    struct timeval timeout;
    packet recv_msg;
    msg_header recv_header;
    packet *send_msg;

    fd_set reads, cpy_reads;

    socklen_t addr_size;
    int fd_max, fd_num, i;
    char buf[BUF_SIZE] = {0, };

    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    signal(SIGINT, (void *)sig_handler);

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1 )
    {
        logging("connection error");
    }
    if(listen(serv_sock, 5)==-1)
    {	
        logging("listen error");
    }

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    while(1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        if((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
            break;

        if(fd_num == 0)
            continue;

        for(i = 0; i < fd_max + 1; ++i)
        {
            if(FD_ISSET(i, &cpy_reads))
            {
                if(i == serv_sock)
                {
                    addr_size = sizeof(clnt_addr);

                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr,&addr_size);

                    printf("client %s conncted\n", inet_ntoa(clnt_addr.sin_addr));

                    FD_SET(clnt_sock, &reads);
                    if(fd_max < clnt_sock)
                        fd_max = clnt_sock;

                    logging(inet_ntoa(clnt_addr.sin_addr));	

                }

                else
                {

                    int str_len = 0;
                    int header_size = sizeof(msg_header);

                    // 헤더를 받음
                    while(str_len < header_size)
                    {
                        str_len = read(i, (void *)&recv_header, header_size);
                    }

                    int packet_size = ntohs(recv_header.packet_size);
                    int msg_type = ntohs(recv_header.msg_type);
                    int body_size = ntohs(recv_header.body_size);

                    // 바디크기 만큼 동적 할당
                    str_len = 0;

                    while(str_len < body_size)
                    {
                        str_len = read(i, (void *)&recv_msg.body, body_size);
                    }

                    recv_msg.header = recv_header;

                    // close 처리
                    if(str_len == 0 || msg_type == LOGOUT_REQ)
                    {
                        FD_CLR(i, &reads);
                        shutdown(i, SHUT_WR);
                        logging("closed");
                    }

                    else
                    {		
                        char *result = 0;

                        int msg = ntohs(recv_msg.header.msg_type);

                        // process message
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
 *  메세지 타입과 버퍼를 입력받아
 *  해당 함수 실행
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

void sig_handler(int signo)
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
