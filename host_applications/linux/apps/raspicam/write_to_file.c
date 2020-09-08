main

    if (parse_cmdline(argc, argv, &state)) static int
    parse_cmdline(int argc, const char **argv,
                  RASPIVID_STATE *state) int parms_used =
        (raspicamcontrol_parse_cmdline(&state->camera_parameters, &argv[i][1],
                                       second_arg));
case CommandOutput: // output filename
{
  int len = strlen(arg2);
  if (len) {
    // Ensure that any %<char> is either %% or %d.
    const char *percent = arg2;

    while (*percent && (percent = strchr(percent, '%')) != NULL) {
      int digits = 0;
      percent++;
      while (isdigit(*percent)) {
        percent++;
        digits++;
      }
      if (!((*percent == '%' && !digits) || *percent == 'd')) {
        used = 0;
        fprintf(stderr, "Filename contains %% characters, but not %%d or %%%% "
                        "- sorry, will fail\n");
        break;
      }
      percent++;
    }

    state->filename = malloc(len + 10); // leave enough space for any timelapse
                                        // generated changes to filename
    vcos_assert(state->filename);
    if (state->filename)
      strncpy(state->filename, arg2, len + 1);
    used = 2;
  } else
    used = 0;
  break;
}

if (state.common_settings.filename) {
  if (state.common_settings.filename[0] == '-') {
    state.callback_data.file_handle = stdout;
  } else {
    state.callback_data.file_handle =
        open_filename(&state, state.common_settings.filename);
    static FILE *open_filename(RASPIVID_STATE * pState, char *filename)
        new_handle = fopen(filename, "wb");
    return new_handle;
  }

  if (!state.callback_data.file_handle) {
    // Notify user, carry on but discarding encoded output buffers
    vcos_log_error(
        "%s: Error opening output file: %s\nNo output file will be generated\n",
        __func__, state.common_settings.filename);
  }
}

// Send all the buffers to the encoder output port
if (state.callback_data.file_handle) {
  int num = mmal_queue_length(state.encoder_pool->queue);
  int q;
  for (q = 0; q < num; q++) {
    MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.encoder_pool->queue);

    if (!buffer)
      vcos_log_error("Unable to get a required buffer %d from pool queue", q);

    if (mmal_port_send_buffer(encoder_output_port, buffer) != MMAL_SUCCESS)
      vcos_log_error("Unable to send a buffer to encoder output port (%d)", q);
  }
}

if (state.callback_data.file_handle &&
    state.callback_data.file_handle != stdout)
  fclose(state.callback_data.file_handle);