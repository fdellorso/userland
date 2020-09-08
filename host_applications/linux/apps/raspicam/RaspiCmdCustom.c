#include "RaspiCmdCustom.h"
#include <time.h>
#include "jsmn/jsmn.h"

int raspicmdcustom_record_video(void *data)
{

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    (void)data;

    printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}