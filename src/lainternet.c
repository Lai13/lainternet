#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <sys/mman.h>

#include "base64.h"

#include "lainternet.h"
#include "Client_List.h"
#include "POP3.h"
#include "SMTP.h"

int tun_id = 0;

int
main (int argc, char * argv[])
{
    /* check if user is root */
    if (geteuid () != 0)
    {
	printf ("Exiting... user is not root!\n");
	return 1;
    }
    
    struct Lainternet_Config config;
    
    config.is_custom_arg = 0;

    /* set path of config file */
    char path[512];
    strcat (strcpy (path, getenv ("HOME")),
	    "/.config/lainternet/lainternet.conf");

    config.location = malloc (strlen (path) * sizeof (char));
    strcpy (config.location, path);
    
    /* read arguments */
    static struct option long_options[] =
    {
	{"config", required_argument, 0, 'c'},
	{0, 0, 0, 0}
    };

    int long_index = 0;
    int opt = 0;

    /* loop through command line arguments */
    while ((opt = getopt_long (argc, argv, "c:", long_options,
			       &long_index)) != -1)
    {
	switch (opt)
	{
	    case 'c':
		config.location = optarg;
		config.is_custom_arg = 1;
		break;
	    default:
		return 1;
	}
    }
    
    /* read config file and parse */
    ready_config (&config);
    
    struct Client_List *  clients;
    clients = malloc (sizeof (struct Client_List));
    client_list_initialize (clients);

    init_pop3 (&config);
    init_smtp (&config);
    
    while (1)
    {
    	struct Request pop3_request;
    	pop3_request = get_oldest_email ();
	printf ("Sender: %s, Message: %s\n", pop3_request.sender,
		pop3_request.message);
    	struct Lainternet_Client c;
    	if (pop3_request.sender != 0)
    	{
    	    if (client_list_get (clients, pop3_request.sender) == 0)
    	    {
    		/* client is new */
    		c.rcpt = pop3_request.sender;
    		char * n = malloc (sizeof (char) * 2);
    		n[0] = ++tun_id;
    		n[1] = '\0';
    		c.interface_fd = get_tun_interface (n);
    	       	client_list_add (clients, pop3_request.sender, &c);
    	    }
    	    else
    	    {
    		c = *(client_list_get (clients, pop3_request.sender));
    	    }

	    char * decoded = malloc (Base64decode_len (pop3_request.message));
	    Base64decode (decoded, pop3_request.message);

    	    write(c.interface_fd, decoded,
    	    	  strlen (decoded));
	    free (decoded);
    	}
    	char packet[65507];

    	for (int i = 0; i < clients->length; i++)
    	{
    	    int len = 0;
    	    struct Lainternet_Client *  c;
    	    if ((c = client_list_iget (clients, i)) == 0)
    		continue;
	    
    	    if ((len = read (c->interface_fd, &packet, sizeof (packet))) > 0)
    	    {
    		struct Send_Request smtp_request;
    		smtp_request.client = *c;
    		int true_length = strlen (packet);
    		smtp_request.message_body = malloc (sizeof (char)
    						    * true_length);
    		strncpy (smtp_request.message_body, packet, true_length);
    		request_send (&smtp_request);
    	    }
    	    else
    	    {
    		continue;
    	    }
    	}
    }
    
    return 0;
}

int
ready_config (struct Lainternet_Config * config)
{
    FILE * config_file;
    config_file = fopen (config->location, "r");
    int parse_result;
    parse_result = parse_config_file (config, config_file);

    /* there was an error in parsing */
    if (parse_result != 0)
    {
	switch (parse_result)
	{
	    case 1:
		printf ("Error opening configuration file\n");
		perror (config->location);
		return 1;
		break;
	    case 2:
		printf ("Configuration file has unrecognized " 
		       "tokens\n");
		return 1;
		break;
	    default:
		printf ("An unknown error has occured when " 
		       "reading the configuration file\n");
		return 1;
		break;
	}
	
    }
    
    fclose (config_file);

    return 0;
}

int
parse_config_file (struct Lainternet_Config * config, FILE * config_file)
{
    /* if file could not be found, return 1 */
    if (config_file == 0)
	return 1;
    
    char line[512];

    /* loop through each line of config file */
    while (fgets (line, sizeof (line), config_file) != 0)
    {
	/* split line at colon (field:value) */
	char * token = strtok (line, " ");
	char ** config_member;
	if (strcmp (token, "email") == 0)
	{
	   config_member = &(config->email);
	}
	else if (strcmp (token, "password") == 0)
	{
	   config_member = &(config->password);
	}
	else if (strcmp (token, "smtp-mail-server") == 0)
	{
	   config_member = &(config->smtp_mail_server);
	}
	else if (strcmp (token, "pop3-mail-server") == 0)
	{
	    config_member = &(config->pop3_mail_server);
	}
	/* field is not recognized */
	else
	{
	    return 2;
	}
	
	/* get second token (value) */
	token = strtok (NULL, " ");
	/* remove new line characters from string */
	token[strcspn (token, "\r\n")] = 0;
	/* initializes char pointer in struct */
	*config_member = malloc (sizeof (char) * strlen (token));
	/* copies token to char pointer in struct */
	strcpy (*config_member, token);
    }
    
    return 0;
}

int
get_tun_interface (char * name)
{
    int interface = open ("/dev/net/tun", O_RDWR | O_NONBLOCK);

    struct ifreq ifr;
    memset (&ifr, 0, sizeof (ifr));

    /* set as TUN device and do not provide packet info */
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    if (ioctl (interface, TUNSETIFF, &ifr) < 0)
    {
	perror ("Error getting TUN interface");
	return -1;
    }

    return interface;

}
