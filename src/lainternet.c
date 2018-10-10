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

#include "lainternet.h"

int
main (int argc, char * argv[])
{
    struct lainternet_config config;
    
    config.is_custom_arg = 0;

    /* set path of config file */
    char path[512];
    strcat (strcpy (file, getenv ("HOME")),
	    "/.config/lainternet/lainternet.conf");

    config.location = malloc (strlen (file) * sizeof (char));
    strcpy (config.location, file);
    
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
    
    return 0;
}

int
get_tun_interface ()
{
    int interface = open ("/dev/net/tun", O_RDWR | O_NONBLOCK);

    struct ifreq ifr;
    memset (&ifr, 0, sizeof (ifr));

    /* set as TUN device and do not provide packet info */
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if (ioctl (interface, TUNSETIFF, &ifr))
    {
	perror ("Error getting TUN interface");
	return -1;
    }

    return interface;
}

int
ready_config (struct lainternet_config * config)
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
parse_config_file (struct lainternet_config * config, FILE * config_file)
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
