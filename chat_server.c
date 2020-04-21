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

void *chat_room_thread(void *arg);
void *client_thread(void *arg);

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
	pthread_mutex_t *lock;
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
	pthread_cond_t cond;
	ChatRoom *chat_room;
	Client *client;

	if(argc < 4) {
		fprintf(stderr, "usage: chat_server\n");
		exit(1);
	}

	jrb = make_jrb();
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);

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
		chat_room->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
		chat_room->lock = &lock;
		chat_room->cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
		chat_room->cond = &cond;
	}


//   jrb_traverse(tmp, jrb) {
//     chat_room = (ChatRoom *) tmp->val.v;
//     printf("%s\n", chat_room->room_name);
//   }

	while(1) {
		fd = accept_connection(sock);
		client = (Client *) malloc(sizeof(Client));
		client->fd = fd;
		pthread_create(&tid, NULL, client_thread, client);
	}
	pthread_exit(NULL);

    return 0;
}

void *chat_room_thread(void *arg) {

	return NULL;
}

void *client_thread(void *arg) {
	Client *client;
	ChatRoom *chat_room;
	JRB tmp;
	client = (Client *) arg;
	client->client_name = malloc(50);

	char *chat_room_name = malloc(100);

	client->fin = fdopen(client->fd, "r");
	client->fout = fdopen(client->fd, "w");

	fputs("Chat Rooms:\n\n", client->fout);
	jrb_traverse(tmp, jrb) {
		chat_room = (ChatRoom *) tmp->val.v;
		fputs(chat_room->room_name, client->fout);
		fputs(":\n", client->fout);
	}
	fputs("\n", client->fout);
	fflush(client->fout);

	fputs("Enter your chat name (no spaces):\n", client->fout);
	fflush(client->fout);
	fgets(client->client_name, 50, client->fin);

	fputs("Enter chat room:\n", client->fout);
	fflush(client->fout);
	fgets(chat_room_name, 100, client->fin);

	return NULL;
}


