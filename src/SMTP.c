#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "SMTP.h"

CURL * curl_smtp;
CURLcode res_smtp = CURLE_OK;
struct Send_Request * request;
struct Upload_Status * upload_ctx;

int
request_send (struct Send_Request * req)
{
    /* initialize upload status or clear it from last time */
    upload_ctx = malloc (sizeof (struct Upload_Status));
    upload_ctx->lines_read = 0;
    
    request = 0;
        
    if (req->client.rcpt == 0)
	return 1;
    if (req->message_body == 0)
	return 2;
    
    struct curl_slist * rcpt = 0;
    
    rcpt = curl_slist_append (rcpt, req->client.rcpt);
    curl_easy_setopt (curl_smtp, CURLOPT_MAIL_RCPT, rcpt);
    
    request = req;

    res_smtp = curl_easy_perform (curl_smtp);

    if (res_smtp != CURLE_OK)
	return -1;

    return 0;
}

int
init_smtp (struct Lainternet_Config * config)
{
    struct Upload_Status upload_stat;
    
    curl_smtp = curl_easy_init ();

    if (!curl_smtp)
	return 1;
    
    /* set login credentials */
    curl_easy_setopt (curl_smtp, CURLOPT_USERNAME, config->email);
    curl_easy_setopt (curl_smtp, CURLOPT_PASSWORD, config->password);
    
    char * protocol = "smtps://";
    
    /* allocate for concatenation of mail server and inbox */
    char * smtp_url = malloc (sizeof (protocol) +
			      sizeof (config->smtp_mail_server));
	
    /* prepend protocol to URL of mail server */
    strcpy (smtp_url, protocol);
    strcpy (smtp_url + strlen(protocol), config->smtp_mail_server);
    
    /* set URL of smtp mail server */
    curl_easy_setopt (curl_smtp, CURLOPT_URL, smtp_url);

    curl_easy_setopt (curl_smtp, CURLOPT_MAIL_FROM, config->email);
    
    curl_easy_setopt (curl_smtp, CURLOPT_READFUNCTION, write_data);
    curl_easy_setopt (curl_smtp, CURLOPT_READDATA, &upload_stat);
    curl_easy_setopt (curl_smtp, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt (curl_smtp, CURLOPT_VERBOSE, 0L);
    
    return 0;
}

size_t
write_data (void * ptr, size_t size, size_t nmemb, void * userp)
{
    const char * data = 0;

     if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
	return 0;
     
    char * message_body[] = {"\r\n", ".", 0};
    message_body[1] = malloc ((strlen (request->message_body) + 2)
			      * sizeof (char));
    strcpy (message_body[1], request->message_body);
    strcpy (message_body[1] + strlen (request->message_body), "\r\n");

    data = message_body[upload_ctx->lines_read];
    if (data)
    {
    	size_t length = strlen (data);
    	memcpy (ptr, data, length);
    	upload_ctx->lines_read++;
	free (message_body[1]);
    	return length;
    }
    
    return 0;
}


