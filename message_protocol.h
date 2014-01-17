#define MAX_DATA 1024

enum msg_type {LS_REQ = 2, LS_RESP, CAT_REQ, CAT_RESP, SAVE_REQ, SAVE_RESP, 
		RM_REQ, RM_RESP,MKDIR_REQ, MKDIR_RESP, RMDIR_REQ, RMDIR_RESP,
		CD_REQ, CD_RESP, CP_REQ, CP_RESP, LOGIN_REQ, LOGIN_RESP,
		LOGOUT_REQ, LOGOUT_RESP, ERROR = -1};

#pragma pack(1)
typedef struct st_message_header {
	int packet_size;
	int msg_type;
	int body_size;
}msg_header;

typedef struct st_message_body {
	char data[MAX_DATA];
}msg_body;

typedef struct st_message {
	msg_header header;
	msg_body body;
}packet;

#pragma pack()
