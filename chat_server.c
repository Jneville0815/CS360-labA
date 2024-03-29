/*  Jimmy Neville
	labA -- chat_server.c
	CS360 -- Systems Programming
	Spring 2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "jrb.h"
#include "dllist.h"
#include "jval.h"
#include "fields.h"

/* Function prototypes */
void *chat_room_thread(void *arg);
void *client_thread(void *arg);

/* Contains information specific to each client */
typedef struct Client {
	char *client_name;
	FILE *fin;
	FILE *fout;
	int fd;
} Client;

/* Contains information specific to each chat room */
typedef struct ChatRoom {
	char *room_name;
	Dllist messages;
	Dllist clients;
	pthread_mutex_t *lock;
	pthread_cond_t *cond;
} ChatRoom;

/*  jrb is a global variable whoese key is the name 
	of a chat room and value is the instance of ChatRoom*
*/
JRB jrb;

int main(int argc, char **argv) {
	JRB tmp;
	pthread_t *tid;
	int sock;
	int fd;
	ChatRoom *chat_room;
	Client *client;

	/* If all command line arguments not specified, error and exit */
	if(argc < 3) {
		fprintf(stderr, "usage: chat_server\n");
		exit(1);
	}

	jrb = make_jrb();

	/* Serve the port specified on command line */
	sock = serve_socket(atoi(argv[1]));

	/*  Create an instance of chat_room for each chat room name 
		specified on the command line. Also initialize/
		malloc the needed instance variables and data structures
		for each chat room. Lastly, create a thread for each
		chat room.
	*/
	pthread_t *chat_room_tids;
	chat_room_tids = malloc(sizeof(chat_room_tids) * (argc - 2));
	int i;
	for(i = 2; i < argc; i++) {
		chat_room = (ChatRoom *) malloc(sizeof(ChatRoom));
		chat_room->room_name = malloc(strlen(argv[i])+1);
		strcpy(chat_room->room_name, argv[i]);
		jrb_insert_str(jrb, 
					   chat_room->room_name, 
					   new_jval_v((void *) chat_room)
		);
		chat_room->messages = new_dllist();
		chat_room->clients = new_dllist();
		chat_room->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
		chat_room->cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
		pthread_mutex_init(chat_room->lock, NULL);
		pthread_cond_init(chat_room->cond, NULL);
		pthread_create(chat_room_tids+(i-2), NULL, chat_room_thread, chat_room);
	}

	/*  Wait for clients to connect to server, and when they do, create 
		a new client thread.
	*/
	while(1) {
		fd = accept_connection(sock);
		client = (Client *) malloc(sizeof(Client));
		client->fd = fd;
		tid = malloc(sizeof(pthread_t));
		pthread_create(tid, NULL, client_thread, client);
	}

    return 0;
}

/*  Thread is blocked on a condition variable. When the 
	condition variable is unblocked, that is the indication 
	that the server has received input from a client. When 
	the chat room thread receives a message, it traverses the 
	list of clients and sends the message to each client.
	After sending the messages it deletes the list of
	messages and makes a new list. When it is done processing 
	the list, it should wait on the condition variable again.
	There is one thread for each chat room.
*/
void *chat_room_thread(void *arg) {
	ChatRoom *chat_room;
	Client *client;
	chat_room = (ChatRoom *) arg;

	Dllist tmp, tmp2;
	char *message;

	pthread_mutex_lock(chat_room->lock);

	while(1) {
		pthread_cond_wait(chat_room->cond, chat_room->lock);

		dll_traverse(tmp, chat_room->messages) {
			message = tmp->val.s;

			dll_traverse(tmp2, chat_room->clients) {
				client = (Client *) tmp2->val.v;
				fputs(message, client->fout);
				fflush(client->fout);
			}

			free(tmp->val.s);
		}

		free_dllist(chat_room->messages);
		chat_room->messages = new_dllist();
	}

	pthread_mutex_unlock(chat_room->lock);

	return NULL;
}

/*  Client thread prints information about the chat room
	and asks user for name and for the chat room they 
	would like to enter. If EOF is discovered at any
	time while reading input, the client will exit.
	Thread will typically be blocked reading from the 
	socket. When it receives a line of text from the 
	socket, it will construct the proper string from it,
	put that string onto the chat room's list, and 
	signal the chat room server. There is one thread for
	each client.
*/
void *client_thread(void *arg) {
	Client *client;
	ChatRoom *chat_room;
	JRB tmp_j;
	Dllist tmp_d;
	client = (Client *) arg;
	client->client_name = malloc(50);

	char *buf = malloc(500);
	char *msg = malloc(500);

	char *chat_room_name = malloc(100);

	client->fin = fdopen(client->fd, "r");
	client->fout = fdopen(client->fd, "w");

	Client *tmp_client;
	fputs("Chat Rooms:\n\n", client->fout);
	jrb_traverse(tmp_j, jrb) {
		chat_room = (ChatRoom *) tmp_j->val.v;
		fputs(chat_room->room_name, client->fout);
		fputs(":", client->fout);
		dll_traverse(tmp_d, chat_room->clients) {
			tmp_client = (Client *) tmp_d->val.v;
			fputs(" ", client->fout);
			fputs(tmp_client->client_name, client->fout);
		}
		fputs("\n", client->fout);
	}
	fputs("\n", client->fout);
	fflush(client->fout);

	fputs("Enter your chat name (no spaces):\n", client->fout);
	fflush(client->fout);
	if(fgets(client->client_name, 50, client->fin) == NULL) {
		fclose(client->fin);
		fclose(client->fout);
		client->fout = NULL;
		pthread_exit(NULL);
	}
	client->client_name[strlen(client->client_name) - 1] = 0;

	fputs("Enter chat room:\n", client->fout);
	fflush(client->fout);
	if(fgets(chat_room_name, 100, client->fin) == NULL) {
		fclose(client->fin);
		fclose(client->fout);
		client->fout = NULL;
		pthread_exit(NULL);		
	}
	chat_room_name[strlen(chat_room_name) - 1] = 0;

	jrb_traverse(tmp_j, jrb) {
		chat_room = (ChatRoom *) tmp_j->val.v;
		if(strcmp(chat_room->room_name, chat_room_name) == 0) {
			sprintf(buf, "%s has joined\n", client->client_name);
			dll_append(chat_room->messages, new_jval_s(strdup(buf)));
			
			pthread_mutex_lock(chat_room->lock);

			dll_append(chat_room->clients, new_jval_v((void *) client));
			pthread_cond_signal(chat_room->cond);
			pthread_mutex_unlock(chat_room->lock);
			break;
		}
	}

	while(1) {
		if(fgets(msg, 500, client->fin) == NULL) {
			sprintf(buf, "%s has left\n", client->client_name);
			fclose(client->fin);

			if(client->fout != NULL) {
				fclose(client->fout);
				client->fout = NULL;

				pthread_mutex_lock(chat_room->lock);

				dll_append(chat_room->messages, new_jval_s(strdup(buf)));

				pthread_cond_signal(chat_room->cond);

				dll_traverse(tmp_d, chat_room->clients) {
					tmp_client = (Client *) tmp_d->val.v;
					if(tmp_client == client) {
						free(client->client_name);
						free(tmp_d->val.v);
						dll_delete_node(tmp_d);
						break;
					}
				}
				pthread_mutex_unlock(chat_room->lock);

				pthread_exit(NULL);
			}
		}

		sprintf(buf, "%s: ", client->client_name);
		strcat(buf, msg);

		pthread_mutex_lock(chat_room->lock);
		dll_append(chat_room->messages, new_jval_s(strdup(buf)));

		pthread_cond_signal(chat_room->cond);
		pthread_mutex_unlock(chat_room->lock);
	}

	return NULL;
}

