#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "POP3.h"

CURL * curl_pop3;
CURLcode res_pop3 = CURLE_OK;

struct Request * current_request;

struct Request
get_oldest_email ()
{
    /* Set up request */
    struct Request request;
    request.sender = 0;
    request.finished = 0;
    request.is_waiting = 0;
    current_request = &request;
    
    /* check if unprocessed emails */
    curl_easy_setopt (curl_pop3, CURLOPT_CUSTOMREQUEST, "RETR 1");
    res_pop3 = curl_easy_perform (curl_pop3);
    current_request = 0;
    curl_easy_setopt (curl_pop3, CURLOPT_CUSTOMREQUEST, "DELE 1");
    res_pop3 = curl_easy_perform (curl_pop3);
    return request;
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
			      sizeof (config->pop3_mail_server));
	
    /* prepend protocol to URL of mail server */
    strcpy (pop3_url, protocol);
    strcpy (pop3_url + strlen(protocol), config->pop3_mail_server);
    
    /* set URL of pop3 mail server */
    curl_easy_setopt (curl_pop3, CURLOPT_URL, pop3_url);

    curl_easy_setopt (curl_pop3, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt (curl_pop3, CURLOPT_WRITEFUNCTION, parse_data_buffer);
    
    return 0;
}



size_t
parse_data_buffer (char * buffer, size_t size, size_t nmemb, void * up)
{
    char current_buffer [MAX_BUFFER];
    
    /* prevents the same request being repeated */
    if (current_request->finished == 1)
	return 0;
    
    /* copy curl buffer into temporary one */
    strncat (current_buffer, buffer, MAX_BUFFER -
	     strlen (current_buffer) - 1); 

    char * line = 0;

    line = strtok (current_buffer, "\r\n");

    /* scan each line, looking for relevant data */
    while (line != 0)
    {
	/* skip blank lines */
	if (line[0] == '\0' || line[0] == '\r')
	    continue;

	/* get sender */
        if (current_request->sender == 0 && strstr (line, "From: ") != 0)
	{
	    char * name = &(line[strlen ("From: ")]);
	    current_request->sender = malloc (strlen (name) * sizeof (char));
	    strcpy (current_request->sender, name);
	}
	    
	/* the plain text is all that is needed */
	if (strstr (line, "<td>") != 0)
	{
	    current_request->is_waiting = 1;
	 
	}
	/* end of message body */
	else if (strstr (line, ";") != 0 && current_request->is_waiting == 1)
	{
	    current_request->is_waiting = 0;
	    current_request->finished = 1;
	    break;
	}
	/* message body */
	else if (current_request->is_waiting == 1)
	{
	    while (line[0] == ' ')
		line = &(line[1]);
	    current_request->request_text = malloc (strlen (line) * sizeof (char));
	    strcpy (current_request->request_text, line);
	}
	
	line = strtok (0, "\r\n");
    }

    return size * nmemb;
}


