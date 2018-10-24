#ifndef LAINTERNET_H
#define LAINTERNET_H

#define PACKET_HEADER "{HDLT:"
#define PACKET_FOOTER "HDLT}"

struct Lainternet_Config
{
    int  is_custom_arg; /* if has been set by custom arguments */
    char * location; /* location of config file */
    char * smtp_mail_server; /* URL for SMTP mail server */
    char * pop3_mail_server; /* URL for IMAP mail server */
    char * email; /* email to be used for data transfer */
    char * password; /* password of email */
};

struct Lainternet_Client
{
    char * rcpt; /* the client's email */
    int interface_fd; /* the client's TUN interface FD */
};

int ready_config (struct Lainternet_Config * config);

int parse_config_file (struct Lainternet_Config * config, FILE * config_file);

int get_tun_interface (char * name);

#endif
