#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "jsmn.h"
#include "RaspiJson.h"

static int jsoneq(char *json, jsmntok_t *tok, char *s)
{
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}

static int jsonext(char *json, jsmntok_t *tok, int r, char *search, char *output)
{
    int i;

    for (i = 1; i < r; i++)
    {
        if (jsoneq(json, &tok[i], search) == 0) {
            /* We may use strndup() to fetch string value */
            sprintf(output, "%.*s\n", tok[i + 1].end - tok[i + 1].start, json + tok[i + 1].start);
            break;
        }
    }

    return 0;
}

// TODO Parametrize json path at startup (-j)
int json_extract(char *search, char *output, char *jsonfile)
{
    int fdjson;
    char buffer[1024];
    int r;
    jsmn_parser json_parser;
    jsmntok_t json_token[128]; /* We expect no more than 128 tokens */

    memset(buffer, '\0', sizeof(buffer));

    if((fdjson = open(jsonfile, O_RDONLY)) == 0) {
        printf("Failed to open JSON\n");
        return -1;
    }
    read(fdjson, buffer, sizeof(buffer));
    close(fdjson);

    jsmn_init(&json_parser);
    r = jsmn_parse(&json_parser, buffer, strlen(buffer), json_token, sizeof(json_token) / sizeof(json_token[0]));
    if (r < 0) {
        printf("Failed to parse JSON: %d\n", r);
        return -1;
    }

    jsonext(buffer, json_token, r, search, output);
    if(!output[0]) {
        printf("Failed to extract JSON: %d\n", r);
        return -1;
    }
    
    return 0;
}