#ifndef LAINTERNET_H
#define LAINTERNET_H

#define DEFAULT_CONFIG_LOCATION "lainternet.conf"

struct lainternet_config
{
    int  is_custom_arg; /* if has been set by custom arguments */
    char * location; /* location of config file */
    char * smtp_mail_server; /* URL for SMTP mail server */
    char * pop3_mail_server; /* URL for IMAP mail server */
    char * email; /* email to be used for data transfer */
    char * password; /* password of email */
};

int ready_config (struct lainternet_config * config);

int parse_config_file (struct lainternet_config * config, FILE * config_file);

int get_tun_interface ();

#endif
