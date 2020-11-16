#include <time.h>
#include <sys/stat.h>

#include "Raspi360.h"
#include "RaspiCmdCustom.h"

#include "RaspiJson.h"

// #include "opencv2/core/core_c.h"
// #include "opencv2/imgproc/imgproc_c.h"

#define VIDEO   0
#define PHOTO   1

static char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos){
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

static char* get_filename(char* filename, int video_photo)
{
    char path[128], prefix[128], datatime[128];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    memset(path, '\0', sizeof(path));
    memset(prefix, '\0', sizeof(prefix));
    memset(datatime, '\0', sizeof(datatime));

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
        if(video_photo)
            strcat(filename, "PHOTO_");
        else
            strcat(filename, "VIDEO_");
    }
    else {
        strcat(filename, prefix);
        strcat(filename, "_");
    }
    strcat(filename, datatime);
    if(!video_photo)
        strcat(filename, ".h264");

    return filename;
}

int raspicmdcustom_record_video(void *data)
{
    char filename[128];

    RASPIVID_STATE *state = (RASPIVID_STATE *) data;

    memset(filename, '\0', sizeof(filename));

    get_filename(filename, VIDEO);

    state->callback_data.record_handle = fopen(filename, "wb");

    if(!state->callback_data.record_handle) {
        printf("Failed to open file to record Video\n");
        return -1;
    }

    fwrite(state->callback_data.header_bytes, 1, state->callback_data.header_wptr, state->callback_data.record_handle);
    // fwrite(state->callback_data.iframe_buff, 1, state->callback_data.iframe_buff_wpos, state->callback_data.record_handle);

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
    char filename[128];
    char file_number[2];
    struct stat st = {0};
    int num, q;

    RASPISTILL_STATE *state = (RASPISTILL_STATE *) data;

    // Register our application with the logging system
    vcos_log_register("RaspiStill", VCOS_LOG_CATEGORY);

    // TODO - Manage filename, direcrories and nr_photos

    state->current_photo += 1;

    memset(filename, '\0', sizeof(filename));

    if(state->common_settings.filename == NULL) {
        get_filename(filename, PHOTO);

        if (stat(filename, &st) == -1) mkdir(filename, 0777);

        state->common_settings.filename = (char *) malloc(strlen(filename) * sizeof(char));

        if (state->common_settings.filename)
            strncpy(state->common_settings.filename, filename, strlen(filename) + 1);
    }
    else {
        strcat(filename, state->common_settings.filename);
    }
    
    strcat(filename, "/");
    sprintf(file_number, "%c", 0x30 + state->current_photo);
    strcat(filename, file_number);
    strcat(filename, ".jpg");

    state->callback_data.file_handle = fopen(filename, "wb");

    // There is a possibility that shutter needs to be set each loop.
    if (mmal_status_to_int(mmal_port_parameter_set_uint32(state->camera_component->control, MMAL_PARAMETER_SHUTTER_SPEED, state->camera_parameters.shutter_speed)) != MMAL_SUCCESS)
        vcos_log_error("Unable to set shutter speed");
    
    // Send all the buffers to the encoder output port
    num = mmal_queue_length(state->encoder_pool->queue);

    for (q=0; q<num; q++)
    {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state->encoder_pool->queue);

        if (!buffer)
        vcos_log_error("Unable to get a required buffer %d from pool queue", q);

        if (mmal_port_send_buffer(state->encoder_component->output[0], buffer)!= MMAL_SUCCESS)
        vcos_log_error("Unable to send a buffer to encoder output port (%d)", q);
    }

    if (mmal_port_parameter_set_boolean(state->camera_component->output[MMAL_CAMERA_CAPTURE_PORT], MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
    {
        vcos_log_error("%s: Failed to start capture", __func__);
    }
    else
    {
        // Wait for capture to complete
        // For some reason using vcos_semaphore_wait_timeout sometimes returns immediately with bad parameter error
        // even though it appears to be all correct, so reverting to untimed one until figure out why its erratic
        vcos_semaphore_wait(&state->callback_data.complete_semaphore);
        if (state->common_settings.verbose) fprintf(stderr, "Finished capture\n");
    }

    fclose(state->callback_data.file_handle);

    // Ensure we don't die if get callback with no open file
    state->callback_data.file_handle = NULL;

    if(state->current_photo == state->total_photos) {
        state->common_settings.filename = NULL;
        state->current_photo = 0;
    }

    return 0;
}

int raspicmdcustom_auto_focus(void *data)
{
    int num, q;

    RASPISTILL_STATE *state = (RASPISTILL_STATE *) data;

    // There is a possibility that shutter needs to be set each loop.
    if (mmal_status_to_int(mmal_port_parameter_set_uint32(state->camera_component->control, MMAL_PARAMETER_SHUTTER_SPEED, state->camera_parameters.shutter_speed) != MMAL_SUCCESS))
        vcos_log_error("Unable to set shutter speed");


    // Send all the buffers to the camera output port
    num = mmal_queue_length(state->raw_pool->queue);

    for (q=0; q<num; q++)
    {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state->raw_pool->queue);

        if (!buffer)
            vcos_log_error("Unable to get a required buffer %d from pool queue", q);

        if (mmal_port_send_buffer(state->camera_component->output[MMAL_CAMERA_PREVIEW_PORT], buffer)!= MMAL_SUCCESS)
            vcos_log_error("Unable to send a buffer to camera output port (%d)", q);
    }

    if (mmal_port_parameter_set_boolean(state->camera_component->output[MMAL_CAMERA_PREVIEW_PORT], MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
    {
        vcos_log_error("%s: Failed to start capture", __func__);
    }
    else
    {
        // Wait for capture to complete
        // For some reason using vcos_semaphore_wait_timeout sometimes returns immediately with bad parameter error
        // even though it appears to be all correct, so reverting to untimed one until figure out why its erratic
        vcos_semaphore_wait(&state->callback_raw_data.complete_semaphore);
        if (state->common_settings.verbose) fprintf(stderr, "Finished capture\n");
    }    

    return 0;
}