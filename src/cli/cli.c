#include "cli.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argtable3.h"


#define DEFAULT_PORT 8080
#define PROGRAM_NAME "WebCerver"
#define PROGRAM_VERSION "1.0.0"


CliResult cli_parse(int argc, char **argv, ServerConf *server_conf) 
{
    struct arg_lit *help;
    struct arg_int *version;
    struct arg_str *subcommand;
    struct arg_int *port;
    struct arg_str *webroot;
    struct arg_end *end;

    void *argtable[] = {
        subcommand = arg_strn(NULL, NULL, "<command>", 0, 1, "Command to execute."),
        port = arg_int0("p", "port", "<port>", "Server port (default: 8080)."),
        webroot = arg_str0("w", "webroot", "<path>", "Web root directory path."),
        help = arg_lit0("h", "help", "print help and exit."),
        version = arg_lit0("v", "version", "print version and exit."),
        end = arg_end(20)
    };

    if (arg_nullcheck(argtable) != 0) 
    {
    fprintf(stderr, "error: insufficient memory for argument table\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return CLI_ERROR;
    }

    int errors = arg_parse(argc, argv, argtable);

    if (help->count > 0)
    {
        cli_print_help();
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return CLI_HELP;
    }


    if (version->count > 0)
    {
        cli_print_version();
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return CLI_HELP; 
    }


    if (errors > 0) 
    {
        arg_print_errors(stderr, end, PROGRAM_NAME);
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return CLI_ERROR;
    }


    if (subcommand->count != 1 || strcmp(subcommand->sval[0], "start") != 0)
    {
        fprintf(stderr, "error: expected command 'start'\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return CLI_ERROR;
    }


    if (webroot->count != 1)
    {
        fprintf(stderr, "error: --webroot is required\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return CLI_ERROR;
    }


    server_conf->port = DEFAULT_PORT;

    if (port->count > 0)
    {
        if (port->ival[0] < 1 || port->ival[0] > 65535) {
            fprintf(stderr, "error: port must be between 1 and 65535\n");
            arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
            return CLI_ERROR;
        }

        server_conf->port = (unsigned short)port->ival[0];
    }

    server_conf->webroot = webroot->sval[0];

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    return CLI_OK;
}


void cli_print_help(void) 
{
    printf(
        "Usage:\n"
        "  %s start --webroot <path> [--port <port>]\n"
        "  %s --help\n"
        "  %s --version\n"
        "\n"
        "Commands:\n"
        "  start                 Start the web server\n"
        "\n"
        "Options:\n"
        "  -p, --port <port>         Server port (default: %d)\n"
        "  -w, --webroot <path>      Web root directory (required)\n"
        "  -h, --help            Display this help and exit\n"
        "  -v, --version             Display version and exit\n",
        PROGRAM_NAME,
        PROGRAM_NAME,
        PROGRAM_NAME,
        DEFAULT_PORT
    );
}


void cli_print_version(void)
{
    printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
}