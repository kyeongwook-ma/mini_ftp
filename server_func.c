#include "server_func.h"

static mfd fd;    
static sock serv_sock;

bool
bind_sock_addr()
{
    return bind(serv_sock.sock_fd, 
        (struct sockaddr *)&serv_sock.sock_addr, 
        sizeof(serv_sock.sock_addr));
}

bool
listen_from_clnt(int capacity)
{
    return listen(serv_sock.sock_fd, capacity);
}

/*
 * set address to socket
 */
void
create_sock(int port)
{
    serv_sock.sock_fd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_sock.sock_addr, 0, sizeof(serv_sock.sock_addr));
    serv_sock.sock_addr.sin_family = AF_INET;
    serv_sock.sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_sock.sock_addr.sin_port = htons(port);
   
}

/*
 * init fd sets
 */
void
init_IO_multiplexing()
{
    FD_ZERO(&fd.reads);
    FD_SET(serv_sock.sock_fd, &fd.reads);
    fd.fd_max = serv_sock.sock_fd;
}

/*
 * half close server
 */
void 
close_serv()
{
    FD_CLR(serv_sock.sock_fd, &fd.reads);
    shutdown(serv_sock.sock_fd, SHUT_WR);
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
accept_clnt_multiplexed()
{
    int clnt_sock;
    sock_addr clnt_addr;
    int clnt_addr_size = sizeof(clnt_addr);
 
 	/* accept client */
    clnt_sock = accept(serv_sock.sock_fd, 
        (struct sockaddr*)&clnt_addr, 
        &clnt_addr_size);

	/* register client to observed socket */
    FD_SET(clnt_sock, &fd.reads);
 
    if(fd.fd_max < clnt_sock)
        fd.fd_max = clnt_sock;

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

void run_server()
{
    while(1) {
    fd.cpy_reads = fd.reads;

        /* set timeout */
        timeval timeout = init_timeout(5, 5000);

        /* IO multiplexing through select() */
        if((fd.fd_num = select(fd.fd_max + 1, &fd.cpy_reads, 0, 0, &timeout)) == -1)
            break;

        /* no connection */
        if(fd.fd_num == 0)
            continue;

        int fd_idx;


        for(fd_idx = 0; fd_idx < fd.fd_max + 1; ++fd_idx) {
            if(FD_ISSET(fd_idx, &fd.cpy_reads)) {
                /* server socket */
                if(fd_idx == serv_sock.sock_fd) {
                    /* accept client */
                    accept_clnt_multiplexed();

                }

                /* receive from client's request */
                else {

                    packet recv_msg, send_msg;
                
                    receive_from_clnt(&recv_msg, fd_idx);

                  
                    /* response to client's request */
                    char *result = 0;

                    int msg_type = ntohs(recv_msg.header.msg_type);

                    /* process message */
                    result = process_message(&recv_msg);

                    send_msg = create_message(result, msg_type);

                    response_to_clnt(&send_msg, fd_idx);
                }
            }
        }
   }
}


void
logging(char *msg)
{
    FILE *fp;
    time_t timer;

    time(&timer);

    fp = fopen("logfile.txt","a");

    fprintf(fp, "%s",  msg);
    fprintf(fp, "%s", ctime(&timer));

    fclose(fp);
}

void *
cmd_cp(void *name_args)
{
    char *name = (char *)name_args;
    char *source;
    char *dest;

    int source_fd;
    int dest_fd;
    int readn;
    int totaln=0;
    char buf[MAXLINE];

    source = strtok(name, " ");
    dest = strtok(NULL, " ");

    if( !(source_fd = open(source, O_RDONLY)))
    {
        return "Error";
    }

    if( !(dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644)))
    {
        return "Error";
    }

    if(errno == EEXIST)
    {
        close(dest_fd);
        return "Error";
    }

    memset(buf, 0x00, MAXLINE);
    while((readn = read(source_fd, buf, MAXLINE)) > 0)
    {
        printf("readn : %d\n", readn);
        totaln+=write(dest_fd, buf, readn);
        memset(buf, 0x00, MAXLINE);                      
    }


    return "copy completed";
}

void *
cmd_cat(void* name_arg)
{
    char *file_name = (char *)name_arg;
    char *buf;
    int size;
    int fd;

    fd = open(file_name,O_RDONLY);
    size = lseek(fd, 0, SEEK_END); // get offset at end of file
    lseek(fd, 0, SEEK_SET); // seek back to beginning
    buf = malloc(size+1); // allocate enough memory.

    read(fd, buf, size);  // read in the file


    close(fd);

    return (char *)buf;
}

void *
cmd_ls(void *path_arg)
{
    DIR *d;
    char *buf;
    char temp[MAXLINE] = {0,};      

    struct dirent *dent;

    d = opendir(".");

    if(!d) 
    {
        return "Can't";
    }       

    while(dent = readdir(d)) {       
        sprintf(temp,"%s", dent->d_name);
        strcat(temp,"\n");
        buf = (char *)malloc(strlen(temp) + 1);
        strcat(buf, temp);
        memset((void *)&temp, 0x00, MAXLINE);
    }

    return buf;
}

void *
cmd_save(void* arg) 
{
    char *msg = (char *)arg;
    char *temp;
    char *file_name;
    char *buf;
    FILE* fp = NULL;

    temp = msg;
    puts(temp);
    int nMsg = strlen(temp) + 1;

    file_name = strtok(temp, " ");

    int nFile_name = strlen(file_name) + 1;
    int nBuf = nMsg - nFile_name;

    buf = (char *)malloc(nBuf + 1);

    int i = 0;

    for(i = 0; i < nBuf; ++i)
    {
        buf[i] = msg[i + nFile_name];
    }

    fp = fopen(file_name,"w");

    if(fp==NULL)
        return "save failed";

    fprintf(fp,"%s",buf);

    fclose(fp);

    strcat(file_name, " is created");

    return file_name;
}

void *
cmd_cd(void *args)
{
    char *arg = (char *)args;
    char *dirinfo;
    char *dirbuf = NULL;


    dirinfo = arg;

    dirbuf = strtok(dirinfo, " ");
    puts(dirbuf);

    //pid = fork();

    if(dirbuf == NULL) 
    {
        chdir(getenv("HOME"));
    }
    else if(dirbuf != NULL)
    {
        chdir(dirbuf);
    }

    else
    {       
        return "Usage cd [dir]";
    }

    return "OK";
}

void *
cmd_rm(void *arg)
{
    char *filename = (char *)arg;
    int result = unlink(filename);

    if(result==0)
        ;
    else if(result ==-1)
        return "rm failed";

    strcat(filename, " is deleted");

    return filename;
}



void *
cmd_rmdir(void *arg)
{
    char *dirname = (char *)arg;

    puts(dirname);

    int result = rmdir(dirname);

    if(result==0)
        printf("rmdir success");
    else if(result ==-1)
        printf("rmdir fail");

    return "deleted";

}



void *
cmd_mkdir(void* arg)
{
    char *dirname = (char *)arg;

    int result = mkdir(dirname, 0777);

    if(result==-1)
        return "mkdir failed";

    strcat(dirname, " is created");

    return dirname;
}



void *
cmd_logout(void *arg)
{
    int sock = *(int *)arg;

    shutdown(sock, SHUT_WR);
}

void *
cmd_login(void *arg)
{
  
}

