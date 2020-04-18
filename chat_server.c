/* When the read end of a pipe or socket goes away, 
fputs() and fflush() will not generate SIGPIPE immediately. 
The bytes are buffered in the operating system and will be 
thrown away eventually. However, the standard I/O calls will 
return without error for a while. Eventually, they will return 
with errors and they may or may not generate SIGPIPE. Your best 
bet is to catch and ignore SIGPIPE and always test return 
values for determine when the read end of the pipe has gone 
away. */

/* For example, in your jtalk_server program, you will have to have
a data structure that holds all of the current connections.
When someone attaches to the socket, you add the connection to
that data structure. When someone quits his/her jtalk session,
then you delete the connection from the data structure. And
when someone sends a line to the server, you will traverse 
the data structure, and send the line to all the connections. 
You will need to protect the data structure with a mutex. 
For example, you do not want to be traversing the data 
structure and deleting a connection at the same time. One
thing you want to think about is how to protect the data 
structure, but at the same time not cause too many threads 
to block on the mutex if they really don't have to. */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "jrb.h"
#include "dllist.h"
#include "jval.h"
#include "fields.h"

void *server_thread(void *arg);

typedef struct Client {
	char *client_name;
	FILE *fin;
	FILE *fout;
	int fd;
} Client;

typedef struct ChatRoom {
	char *room_name;
	Dllist messages;
	Dllist clients;
	pthread_mutex_t *mutex;
	pthread_cond_t *cond;
} ChatRoom;

/*  jrb is a global variable whoese key is the name 
	of chat room and value is the instance of ChatRoom*
*/
JRB jrb;

int main(int argc, char **argv) {
	JRB tmp;
	pthread_t tid;
	int sock;
	int fd;
	pthread_mutex_t lock;
	ChatRoom *chat_room;

	if(argc < 4) {
		fprintf(stderr, "usage: chat_server\n");
		exit(1);
	}

	jrb = make_jrb();
	pthread_mutex_init(&lock, NULL);

	sock = serve_socket(atoi(argv[2]));

	/*  Create an instance of chat_room for each chat room name 
		specified on the command line. Also initialize/
		malloc the needed instance variables and data structures
		for each chat room.
	*/
	int i;
	for(i = 3; i < argc; i++) {
		chat_room = (ChatRoom *) malloc(sizeof(ChatRoom));
		chat_room->room_name = malloc(strlen(argv[i]) + 1);
		strcpy(chat_room->room_name, argv[i]);
		jrb_insert_str(jrb, 
					   chat_room->room_name, 
					   new_jval_v((void *) chat_room)
		);
		chat_room->messages = new_dllist();
		chat_room->clients = new_dllist();
		chat_room->mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
		chat_room->cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t *));
	}

  jrb_traverse(tmp, jrb) {
    chat_room = (ChatRoom *) tmp->val.v;
    printf("%s\n", chat_room->room_name);
  }

    return 0;
}

void *server_thread(void *arg) {

	return NULL;
}


