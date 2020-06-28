#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "linenoise.h"
#include "sds.h"
#include "anet.h"
#include "hiredis.h"

/* cliConnect() flags. */
#define CC_FORCE (1<<0)         /* Re-connect if already connected. */
#define CC_QUIET (1<<1)         /* Don't log connecting errors. */

#define REDIS_CLI_KEEPALIVE_INTERVAL 15 /* seconds */

static redisContext *context;
static struct config {
    char *hostip;
    int hostport;
    char *hostsocket;
    long repeat;
    long interval;
    char prompt[128];
} config;

void completion(const char *buf, linenoiseCompletions *lc) {
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc,"hello");
        linenoiseAddCompletion(lc,"hello there");
    }else if(buf[0] == 's'){
        linenoiseAddCompletion(lc,"start1024udp");
        linenoiseAddCompletion(lc,"stop1024udp");
    }else if(buf[0] == 'r'){
        linenoiseAddCompletion(lc,"read");
    }else if(buf[0] == 'c'){
        linenoiseAddCompletion(lc,"clear");
        linenoiseAddCompletion(lc,"connect");
    }
}

char *hints(const char *buf, int *color, int *bold) {
    if (!strcasecmp(buf,"hello")) {
        *color = 35;
        *bold = 0;
        return " World";
    }else if (!strcasecmp(buf,"start")){
        *color = 35;
        *bold = 0;
        return " 1024udp";
    }else if (!strcasecmp(buf,"stop")) {
        *color = 35;
        *bold = 0;
        return " 1024udp";
    }
    return NULL;
}


static sds *cliSplitArgs(char *line, int *argc) {
    if ((strstr(line,"eval ") == line ||
                            strstr(line,"e ") == line))
    {
        sds *argv = sds_malloc(sizeof(sds)*2);
        *argc = 2;
        int len = strlen(line);
        int elen = line[1] == ' ' ? 2 : 5; /* "e " or "eval "? */
        argv[0] = sdsnewlen(line,elen-1);
        argv[1] = sdsnewlen(line+elen,len-elen);
        return argv;
    } else {
        return sdssplitargs(line,argc);
    }
}

static void cliRefreshPrompt(void) {

    sds prompt = sdsempty();
    if (config.hostsocket != NULL) {
        prompt = sdscatfmt(prompt,"hello %s",config.hostsocket);
    } else {
        char addr[256];
        anetFormatAddr(addr, sizeof(addr), config.hostip, config.hostport);
        prompt = sdscatlen(prompt,addr,strlen(addr));
    }

    /* Copy the prompt in the static buffer. */
    prompt = sdscatlen(prompt,"> ",2);
    snprintf(config.prompt,sizeof(config.prompt),"%s",prompt);
    sdsfree(prompt);
}

static int cliConnect(int flags) {
    if (context == NULL || flags & CC_FORCE) {
        if (context != NULL) {
            redisFree(context);
        }

        if (config.hostsocket == NULL) {
            context = redisConnect(config.hostip,config.hostport);
        } else {
            //context = redisConnectUnix(config.hostsocket);
        }

        if (context->err) {
            if (!(flags & CC_QUIET)) {
                fprintf(stderr,"Could not connect to Redis at ");
                if (config.hostsocket == NULL)
                    fprintf(stderr,"%s:%d: %s\n",
                        config.hostip,config.hostport,context->errstr);
                else
                    fprintf(stderr,"%s: %s\n",
                        config.hostsocket,context->errstr);
            }
            redisFree(context);
            context = NULL;
            return REDIS_ERR;
        }

        /* Set aggressive KEEP_ALIVE socket option in the Redis context socket
         * in order to prevent timeouts caused by the execution of long
         * commands. At the same time this improves the detection of real
         * errors. */
        anetKeepAlive(NULL, context->fd, REDIS_CLI_KEEPALIVE_INTERVAL);

    }
    return REDIS_OK;
}

static void repl(void) {
    sds historyfile = NULL;
    int history = 0;
    char *line;
    int argc;
    sds *argv;

    linenoiseSetMultiLine(1);
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);

    history = 1;
    linenoiseHistoryLoad("history.txt");

    cliRefreshPrompt();
    while((line = linenoise(context ? config.prompt : "not connected> ")) != NULL) {
        if (line[0] != '\0') {
            long repeat = 1;
            int skipargs = 0;
            char *endptr = NULL;

            argv = cliSplitArgs(line,&argc);

            /* check if we have a repeat command option and
             * need to skip the first arg */
            if (argv && argc > 0) {
                errno = 0;
                repeat = strtol(argv[0], &endptr, 10);
                if (argc > 1 && *endptr == '\0') {
                    if (errno == ERANGE || errno == EINVAL || repeat <= 0) {
                        fputs("Invalid redis-cli repeat command option value.\n", stdout);
                        sdsfreesplitres(argv, argc);
                        linenoiseFree(line);
                        continue;
                    }
                    skipargs = 1;
                } else {
                    repeat = 1;
                }
            }

            if (argv == NULL) {
                printf("Invalid argument(s)\n");
                linenoiseFree(line);
                continue;
            } else if (argc > 0) {
                if (strcasecmp(argv[0],"quit") == 0 ||
                    strcasecmp(argv[0],"exit") == 0)
                {
                    exit(0);
                } else if (argc == 3 && !strcasecmp(argv[0],"connect")) {
                    printf("echo: '%s'\n", argv[0]);
                    sdsfree(config.hostip);
                    config.hostip = sdsnew(argv[1]);
                    config.hostport = atoi(argv[2]);
                    cliRefreshPrompt();
                    cliConnect(CC_FORCE);
                } else if (argc == 1 && !strcasecmp(argv[0],"clear")) {
                    linenoiseClearScreen();
                }else if (!strcasecmp(argv[0],"start1024udp")){
                    printf("echo: '%s'\n", argv[0]);
                } else if (!strcasecmp(argv[0],"stop1024udp")){
                    printf("echo: '%s'\n", argv[0]);
                } else if (!strcasecmp(argv[0],"read")){
                    printf("echo: '%s'\n", argv[0]);
                }

                linenoiseHistoryAdd(argv[0]); /* Add to the history. */
                linenoiseHistorySave("history.txt"); /* Save the history on disk. */
            }
            /* Free the argument vector */
            sdsfreesplitres(argv,argc);
        }
        /* linenoise() returns malloc-ed lines like readline() */
        linenoiseFree(line);
    }
    exit(0);
}

int main(int argc, char **argv) {

    config.hostip = sdsnew("127.0.0.1");
    config.hostport = 12345;
    config.hostsocket = NULL;
    config.repeat = 1;
    config.interval = 0;


    char *line;
    char *prgname = argv[0];

    /* Parse options, with --multiline we enable multi line editing. */
    while(argc > 1) {
        argc--;
        argv++;
        if (!strcmp(*argv,"--multiline")) {
            linenoiseSetMultiLine(1);
            printf("Multi-line mode enabled.\n");
        } else if (!strcmp(*argv,"--keycodes")) {
            linenoisePrintKeyCodes();
            exit(0);
        } else {
            fprintf(stderr, "Usage: %s [--multiline] [--keycodes]\n", prgname);
            exit(1);
        }
    }

    repl();

    return 0;
}
