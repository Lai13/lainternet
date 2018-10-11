#ifndef SMTP_H
#define SMTP_H

#include "lainternet.h"

struct send_request
{
    struct lainternet_client client;
    char * message_body;
};

struct upload_status
{
    int lines_read;
};

int init_smtp (struct lainternet_config * config);
int request_send (struct send_request * request);
size_t write_data (void * ptr, size_t size, size_t nmemb, void *userp);

#endif
