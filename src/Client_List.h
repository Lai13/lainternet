#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

#include "lainternet.h"

struct Client_List
{
    int length;
    char ** keys;
    struct Lainternet_Client * values;
};

int client_list_initialize (struct Client_List * list);
int client_list_add (struct Client_List * list, char * key, struct Lainternet_Client * value);
struct Lainternet_Client * client_list_get (struct Client_List * list, char * key);
struct Lainternet_Client * client_list_iget (struct Client_List * list, int index);
int client_list_geti (struct Client_List * list, char * key);
int client_list_remove (struct Client_List * list, char * key);

#endif
