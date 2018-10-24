#ifndef POP3_H
#define POP3_H

#define MAX_BUFFER 9048

#include "lainternet.h"

struct Raw_Message_Data
{
    char * memory;
    size_t size;
};

struct Request
{
    int is_waiting;
    char * header;
    char * message;
    size_t length;
    char * sender;
};

int init_pop3 (struct Lainternet_Config * config);

size_t write_memory_cb (char * buffer, size_t size, size_t nmemb, void * up);

int parse_memory (struct Request * request);

struct Request get_oldest_email ();

#endif
