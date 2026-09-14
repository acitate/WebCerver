#ifndef __CLI_H
#define __CLI_H

/**
 * @brief Struct for storing the program's configuration. 
 * port: the network port to open the server on.
 * webroot: path to the web root directory.
 * 
 */
typedef struct
{
    unsigned short port;
    const char *webroot;
} ServerConf;


typedef enum
{
    CLI_OK = 0,
    CLI_HELP,
    CLI_VERSION,
    CLI_ERROR
} CliResult;


CliResult cli_parse(int argc, char **argv, ServerConf *server_conf);
void cli_print_help(void);
void cli_print_version(void);


#endif