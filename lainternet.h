#ifndef LAINTERNET_H
#define LAINTERNET_H

#define DEFAULT_CONFIG_LOCATION "lainternet.conf"

struct lainternet_config
{
    int  is_custom_arg; /* if has been set by custom arguments */
    char * location; /* location of config file */
    char * email; /* email to be used for data transfer */
    char * password; /* password of email */
};

#endif
