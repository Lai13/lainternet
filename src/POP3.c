#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "POP3.h"

CURL * curl_pop3;
CURLcode res_pop3 = CURLE_OK;

struct Raw_Message_Data data_m;

struct Request
get_oldest_email ()
{
    /* Set up request */
    struct Request request;
    request.sender = 0;
    request.header = 0;
    request.is_waiting = 0;
    request.message = 0;
    request.length = 0;
    
    data_m.memory = malloc (1);
    data_m.size = 0;
    
    /* check if unprocessed emails */
    curl_easy_setopt (curl_pop3, CURLOPT_CUSTOMREQUEST, "TOP 1 500");
    curl_easy_setopt(curl_pop3, CURLOPT_NOBODY, 0L);
    res_pop3 = curl_easy_perform (curl_pop3);

    if (res_pop3 == CURLE_OK)
	parse_memory (&request);
    
    /* delete read email */
    curl_easy_setopt (curl_pop3, CURLOPT_CUSTOMREQUEST, "DELE 1");
    curl_easy_setopt(curl_pop3, CURLOPT_NOBODY, 1L);
    res_pop3 = curl_easy_perform (curl_pop3);

    free (data_m.memory);
    
    return request;
}

int
parse_memory (struct Request * request)
{
    char * line = 0;
    char ** message_content = malloc (1);
    int message_lines = 0;
    
    /* split memory into lines */
    line = strtok (data_m.memory, "\r\n");
    while (line != 0)
    {
	/* get sender */
	if (strstr (line, "From: "))
	{
	    char * sender = line + 6;
	    request->sender = malloc (sizeof (char) * strlen (sender));
	    strcpy (request->sender, sender);
	}
	/* check for start of message */
	else if (strstr (line, PACKET_HEADER))
	{
	    request->is_waiting = 1;
	    char * header = line + strlen (PACKET_HEADER) + 1;
	    request->header = malloc (sizeof (char) * strlen (header));
	    strcpy (request->header, header);
	}
	/* check for end of message */
	else if (strstr (line, PACKET_FOOTER) && request->is_waiting)
	{
	    request->is_waiting = 0;
	    break;
	}
	/* capture message body */
	else if (request->is_waiting)
	{
	    message_content = realloc (message_content, (message_lines + 1) *
				       sizeof (char *));
	    message_content[message_lines] = malloc (sizeof (char) * strlen (line));
	    strcpy (message_content[message_lines], line);
	    message_lines++;
	}
	
	/* advance token */
	line = strtok (0, "\r\n");
    }
    
    //now to clean it up and put into a single string
    request->message = malloc (1);
    request->length = 1;
    for (int i = 0; i < (message_lines); i++)
    {
	
	if (i < (message_lines-3))
	{
	    message_content[i][strlen (message_content[i]) - 1] = '\0';

	}
	else
	{
	    char * p = strstr (message_content[i], "=0");
	    if (p != 0)
	    {
		*p = '\0';
	    }
	}
	request->message = realloc (request->message, request->length + (strlen(message_content[i]) + 1));
	strcpy (request->message + (request->length * (i==0 ? 0 : 1)), message_content[i]);
	request->length = strlen (request->message);
    }

    

    free (message_content);

    return 0;
    
}

int
init_pop3 (struct Lainternet_Config * config)
{
    curl_pop3 = curl_easy_init ();

    if (!curl_pop3)
	return 1;
    
    /* set login credentials */
    curl_easy_setopt (curl_pop3, CURLOPT_USERNAME, config->email);
    curl_easy_setopt (curl_pop3, CURLOPT_PASSWORD, config->password);
    
    char * protocol = "pop3s://";
    
    /* allocate for concatenation of mail server and inbox */
    char * pop3_url = malloc (sizeof (protocol) +
			      sizeof (config->pop3_mail_server) + 2);	
    /* prepend protocol to URL of mail server */
    strcpy (pop3_url, protocol);
    strcpy (pop3_url + strlen (protocol), config->pop3_mail_server);

    /* set URL of pop3 mail server */
    curl_easy_setopt (curl_pop3, CURLOPT_URL, pop3_url);
    curl_easy_setopt (curl_pop3, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt (curl_pop3, CURLOPT_WRITEFUNCTION, write_memory_cb);
    curl_easy_setopt (curl_pop3, CURLOPT_WRITEDATA, (void *) &data_m);
    curl_easy_setopt (curl_pop3, CURLOPT_FORBID_REUSE, 1L);
    
    return 0;
}

size_t
write_memory_cb (char * buffer, size_t size, size_t nmemb, void * up)
{
    size_t data_size = size * nmemb;
    struct Raw_Message_Data * msg_m = (struct Raw_Message_Data *) up;

    char * ptr = realloc (msg_m->memory, msg_m->size + data_size + 1);
    
    if (ptr == 0)
    {
	printf ("Error reallocating in write memory cb!\n");
	return 0;
    }

    msg_m->memory = ptr;
    memcpy (&(msg_m->memory[msg_m->size]), buffer, data_size);
    msg_m->size += data_size;
    msg_m->memory[msg_m->size] = 0;

    return data_size;
}


