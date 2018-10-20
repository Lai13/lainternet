#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Client_List.h"

int
client_list_intitialize (struct Client_List * list)
{
    list->length = 0;
    return list->length;
}

int
client_list_add (struct Client_List * list, char * key, struct Lainternet_Client * value)
{
    int length = list->length;

    if (length == 0)
    {
	/* for efficiency and proper initialization */
	/* allocate char pointer pointer */
	list->keys = malloc (sizeof (char *));
	/* allocate memory for string and copy*/
	list->keys[length] = malloc (sizeof (char) * strlen (key));
	strcpy (list->keys[length], key);
	/* allocate memory for value items */
	list->values = malloc (sizeof (struct Lainternet_Client));
	list->values[length] = *value;
	length++;
    }
    else if (length > 0)
    {
	/* create temporary place for old data */
	char * keys_backup [length];
	struct Lainternet_Client values_backup [length];
	/* copy old data */
	memcpy (keys_backup, list->keys, sizeof (char *) * length);
	memcpy (values_backup, list->values, sizeof (struct Lainternet_Client) * length);
	/* free memory before reassigning pointer */
	free (list->keys);
	free (list->values);
	/* allocate more memory than before for new data */
	list->keys = malloc (sizeof (char *) * (length + 1));
	list->values = malloc (sizeof (struct Lainternet_Client) * (length +1));
	/* copy old data from temporary */
	memcpy (list->keys, keys_backup, sizeof (char *) * length);
	memcpy (list->values, values_backup, sizeof (struct Lainternet_Client) * length);
	length++;
	/* allocate memory for new key string */
	list->keys[length - 1] = malloc (sizeof (char) * strlen (key));
	/* copy new data */
	strcpy  (list->keys[length - 1], key);
	memcpy (list->values + (length - 1), value, sizeof (struct Lainternet_Client));
    }
    
    list->length = length;
    return length;
}

int
client_list_remove (struct Client_List * list, char * key)
{
    int index = 0;
    if ((index = client_list_geti (list, key)) == -1)
	return -1;
    int length = list->length;

    /* create temporary place for data */
    char * keys_backup[length];
    struct Lainternet_Client * values_backup[length];
    /* copy data */
    memcpy (keys_backup, list->keys, sizeof (char *) * length);
    memcpy (values_backup, list->values, sizeof (struct Lainternet_Client) * length);
    /* free memory before reassignment of pointer */
    free (list->keys);
    free (values_backup);
    /* allocate to size one less than before */
    list->keys = malloc (sizeof (char *) * (length - 1));
    list->values = malloc (sizeof (struct Lainternet_Client) * (length - 1));
    /* copy data before item to be removed */
    memcpy (list->keys, keys_backup, sizeof (char *) * index);
    memcpy (list->values, values_backup, sizeof (struct Lainternet_Client) * index);
    /* copy data after item to be removed */
    int size_2 = length - (index + 1);
    memcpy (list->keys + index, keys_backup + index, sizeof (char *) * size_2);
    memcpy (list->values + index, values_backup + index, sizeof (struct Lainternet_Client) * size_2);
    list->length = --length;
    return length;
}

struct Lainternet_Client *
client_list_get (struct Client_List * list, char * key)
{
    for (int i = 0; i < list->length; i++)
    {
	if (strcmp (list->keys[i], key) == 0)
	{
	    return &list->values[i];
	}
    }
    
    return 0;
}

struct Lainternet_Client *
client_list_iget (struct Client_List * list, int index)
{
    struct Lainternet_Client * c = 0;
    if ((c = &list->values[index]))
	return &list->values[index];
    return 0;
}

int
client_list_geti (struct Client_List * list, char * key)
{
    for (int i = 0; i < list->length; i++)
    {
	if (strcmp (list->keys[i], key) == 0)
	{
	    return i;
	}
    }
    
    return -1;
}
