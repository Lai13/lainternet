#ifndef SMTP_H
#define SMTP_H

#include "lainternet.h"

struct Send_Request
{
    struct Lainternet_Client client;
    char * message_body;
};

struct Upload_Status
{
    int lines_read;
};

int init_smtp (struct Lainternet_Config * config);
int request_send (struct Send_Request * request);
size_t write_data (void * ptr, size_t size, size_t nmemb, void *userp);

#endif
