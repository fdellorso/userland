#include <time.h>

#include "RaspiVid.h"
#include "RaspiCmdCustom.h"

#include "RaspiJson.h"

static char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos){
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

int raspicmdcustom_record_video(void *data)
{
    char path[128], prefix[128], datatime[128], filename[128];

    RASPIVID_STATE *state = (RASPIVID_STATE *) data;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    memset(path, '\0', sizeof(filename));
    memset(prefix, '\0', sizeof(filename));
    memset(datatime, '\0', sizeof(filename));
    memset(filename, '\0', sizeof(filename));

    if(json_extract("path", path, "/home/dietpi/rec360/config/rec360_system.json")) {
        printf("Failed to manage JSON\n");
    }
    replace_char(path, '\n', '\0');

    if(json_extract("prefix", prefix, "/home/dietpi/rec360/config/rec360_system.json")) {
        printf("Failed to manage JSON\n");
    }
    replace_char(prefix, '\n', '\0');

    sprintf(datatime, "%d%02d%02d_%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    strcat(filename, path);
    if(prefix[0] == '\0') {
        strcat(filename, "VIDEO_");
    }
    else {
        strcat(filename, prefix);
    }
    strcat(filename, datatime);
    strcat(filename, ".h264");

    state->callback_data.record_handle = fopen(filename, "wb");

    if(!state->callback_data.record_handle) {
        printf("Failed to open file to record Video\n");
        return -1;
    }

    fwrite(state->callback_data.header_bytes, 1, state->callback_data.header_wptr, state->callback_data.record_handle);
    fwrite(state->callback_data.iframe_buff, 1, state->callback_data.iframe_buff_wpos, state->callback_data.record_handle);

    // fclose(state->callback_data.record_handle);

    return 0;
}

int raspicmdcustom_stop_video(void *data)
{
    RASPIVID_STATE *state = (RASPIVID_STATE *) data;

    if(fclose(state->callback_data.record_handle)) {
        state->callback_data.record_handle = NULL;
        printf("Failed to open file to record Video\n");
        return -1;
    }

    state->callback_data.record_handle = NULL;

    return 0;
}

int raspicmdcustom_take_picture(void *data)
{

}
