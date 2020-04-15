/* When the read end of a pipe or socket goes away, 
fputs() and fflush() will not generate SIGPIPE immediately. 
The bytes are buffered in the operating system and will be 
thrown away eventually. However, the standard I/O calls will 
return without error for a while. Eventually, they will return 
with errors and they may or may not generate SIGPIPE. Your best 
bet is to catch and ignore SIGPIPE and always test return 
values for determine when the read end of the pipe has gone 
away. */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "jrb.h"
#include "dllist.h"
#include "jval.h"
#include "fields.h"

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

JRB jrb;

int main(int argc, char **argv) {
    


    return 0;
}


