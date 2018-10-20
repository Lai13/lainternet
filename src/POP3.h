#ifndef POP3_H
#define POP3_H

#define MAX_BUFFER 9048

#include "lainternet.h"

int init_pop3 (struct Lainternet_Config * config);

size_t parse_data_buffer (char * buffer, size_t size, size_t nmemb, void * up);;

struct Request get_oldest_email ();

struct Request
{
    int finished;
    int is_waiting;
    char * request_text;
    char * sender;
};

#endif
