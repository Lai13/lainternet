#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "lainternet.h"

int
main (int argc, char * argv[])
{
    struct lainternet_config config;
    config.is_custom_arg = 2;
    config.location = DEFAULT_CONFIG_LOCATION;
    
    /* Check arguments */
    static struct option long_options[] =
    {
	{"config", required_argument, 0, 'c'},
	{0, 0, 0, 0}
    };

    int long_index = 0;
    int opt = 0;
    
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
		exit (EXIT_FAILURE);
	}
    }
    
    /* read config file and parse */
    FILE * config_file;
    config_file = fopen (config.location, "r");
    int parse_result;
    if ((parse_result = parse_config_file (&config, config_file)) != 0)
    {
	switch (parse_result)
	{
	    case 1:
		printf ("Error reading configuration file\n");
		perror (config.location);
		return 1;
		break;
	    case 2:
		printf ("Configuration file has unrecognized tokens\n");
		return 1;
		break;
	    default:
		printf ("An unknown error has occured when" 
		       "reading the configuration file\n");
		return 1;
		break;
	}
	
    }
    
    fclose (config_file);

    printf ("Email: %s Password: %s\n", config.email, config.password);
    
    return 0;
}

int
parse_config_file (struct lainternet_config * config, FILE * config_file)
{
    /* if file could not be found, return 1 */
    if (config_file == 0)
	return 1;
    
    char line[248];

    /* loop through each line of config file */
    while (fgets (line, sizeof (line), config_file) != 0)
    {
	/* split line at colon (field:value) */
	char * token = strtok (line, ":");

	if (strcmp (token, "email") == 0)
	{
	   /* get second token (value) */
	   token = strtok (NULL, ":");
	   /* remove new line characters from string */
	   token[strcspn (token, "\r\n")] = 0;
	   config->email = malloc (sizeof (token));
	   strcpy (config->email, token);
	}
	else if (strcmp (token, "password") == 0)
	{
	   /* get second token (value) */
	   token = strtok (NULL, ":");
	   /* remove new line characters from string */
	   token[strcspn (token, "\r\n")] = 0;
	   config->password = malloc (sizeof (token));
	   strcpy (config->password, token);
	}
	else
	{
	    return 2;
	}
    }
    
    return 0;
}
