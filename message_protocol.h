#define MAX_DATA 1024


#pragma pack(1)
typedef struct st_message_header {
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
