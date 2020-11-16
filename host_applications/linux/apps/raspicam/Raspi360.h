#include <stdio.h>
#include <stdbool.h>

#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_connection.h"

#include "RaspiCommonSettings.h"
#include "RaspiCamControl.h"
#include "RaspiPreview.h"
#include "RaspiHelpers.h"

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Forward
typedef struct RASPIVID_STATE_S RASPIVID_STATE;

/** Struct used to pass information in encoder port userdata to callback
 */
typedef struct
{
   FILE *file_handle;                   /// File handle to write buffer data to.
   FILE *record_handle;    /// File handle to record video to file.
   int start_record;
   RASPIVID_STATE *pstate;              /// pointer to our state in case required in callback
   int abort;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
   // char *cb_buff;                       /// Circular buffer
   // int   cb_len;                        /// Length of buffer
   // int   cb_wptr;                       /// Current write pointer
   // int   cb_wrap;                       /// Has buffer wrapped at least once?
   // int   cb_data;                       /// Valid bytes in buffer
// #define IFRAME_BUFSIZE (60*1000)
//    int   iframe_buff[IFRAME_BUFSIZE];          /// buffer of iframe pointers
//    int   iframe_buff_wpos;
//    int   iframe_buff_rpos;
   char  header_bytes[29];
   int  header_wptr;
   // FILE *imv_file_handle;               /// File handle to write inline motion vectors to.
   // FILE *raw_file_handle;               /// File handle to write raw data to.
   int  flush_buffers;
   // FILE *pts_file_handle;               /// File timestamps
} PORT_USERDATA;

/** Possible raw output formats
 */
typedef enum
{
   RAW_OUTPUT_FMT_YUV = 0,
   RAW_OUTPUT_FMT_RGB,
   RAW_OUTPUT_FMT_GRAY,
} RAW_OUTPUT_FMT;

/** Structure containing all state information for the current run
 */
struct RASPIVID_STATE_S
{
   RASPICOMMONSETTINGS_PARAMETERS common_settings;     /// Common settings
   int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   MMAL_FOURCC_T encoding;             /// Requested codec video encoding (MJPEG or H264)
   int bitrate;                        /// Requested bitrate
   int framerate;                      /// Requested frame rate (fps)
   int intraperiod;                    /// Intra-refresh period (key frame rate)
   int quantisationParameter;          /// Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
   int bInlineHeaders;                  /// Insert inline headers to stream (SPS, PPS)
   // int demoMode;                       /// Run app in demo mode
   // int demoInterval;                   /// Interval between camera settings changes
   int immutableInput;                 /// Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
   /// the camera output or the encoder output (with compression artifacts)
   int profile;                        /// H264 profile to use for encoding
   int level;                          /// H264 level to use for encoding
   int waitMethod;                     /// Method for switching between pause and capture

   int onTime;                         /// In timed cycle mode, the amount of time the capture is on per cycle
   int offTime;                        /// In timed cycle mode, the amount of time the capture is off per cycle

   // int segmentSize;                    /// Segment mode In timed cycle mode, the amount of time the capture is off per cycle
   // int segmentWrap;                    /// Point at which to wrap segment counter
   // int segmentNumber;                  /// Current segment counter
   // int splitNow;                       /// Split at next possible i-frame if set to 1.
   // int splitWait;                      /// Switch if user wants splited files

   RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   // MMAL_COMPONENT_T *splitter_component;  /// Pointer to the splitter component
   MMAL_COMPONENT_T *encoder_component;   /// Pointer to the encoder component
   // MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera or splitter to preview
   // MMAL_CONNECTION_T *splitter_connection;/// Pointer to the connection from camera to splitter
   MMAL_CONNECTION_T *encoder_connection; /// Pointer to the connection from camera to encoder

   // MMAL_POOL_T *splitter_pool; /// Pointer to the pool of buffers used by splitter output port 0
   MMAL_POOL_T *encoder_pool; /// Pointer to the pool of buffers used by encoder output port

   PORT_USERDATA callback_data;        /// Used to move data to the encoder callback

   int bCapturing;                     /// State of capture/pause
   // int bCircularBuffer;                /// Whether we are writing to a circular buffer

   // int inlineMotionVectors;             /// Encoder outputs inline Motion Vectors
   // char *imv_filename;                  /// filename of inline Motion Vectors output
   // int raw_output;                      /// Output raw video from camera as well
   // RAW_OUTPUT_FMT raw_output_fmt;       /// The raw video format
   // char *raw_filename;                  /// Filename for raw video output
   int intra_refresh_type;              /// What intra refresh type to use. -1 to not set.
   int frame;
   // char *pts_filename;
   // int save_pts;
   // int64_t starttime;
   // int64_t lasttime;

   int fd;
   int epoll_fd;

   bool netListen;
   // MMAL_BOOL_T addSPSTiming;
   int slices;

   char *json_file;
};

// Forward
typedef struct RASPISTILL_STATE_S RASPISTILL_STATE;

/** Struct used to pass information in encoder port userdata to callback
 */
typedef struct
{
   FILE *file_handle;                   /// File handle to write buffer data to.
   uint8_t *raw_buffer;
   int buffer_pointer; 
   VCOS_SEMAPHORE_T complete_semaphore; /// semaphore which is posted when we reach end of frame (indicates end of capture or fault)
   RASPISTILL_STATE *pstate;            /// pointer to our state in case required in callback
} PORT_STILL_USERDATA;

/** Structure containing all state information for the current run
 */
struct RASPISTILL_STATE_S
{
   RASPICOMMONSETTINGS_PARAMETERS common_settings;     /// Common settings
   int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int quality;                        /// JPEG quality setting (1-100)
   int wantRAW;                        /// Flag for whether the JPEG metadata also contains the RAW bayer image
   char *linkname;                     /// filename of output file
   int frameStart;                     /// First number of frame output counter
   MMAL_PARAM_THUMBNAIL_CONFIG_T thumbnailConfig;
   // int demoMode;                       /// Run app in demo mode
   // int demoInterval;                   /// Interval between camera settings changes
   MMAL_FOURCC_T encoding;             /// Encoding to use for the output file.
   // const char *exifTags[MAX_USER_EXIF_TAGS]; /// Array of pointers to tags supplied from the command line
   int numExifTags;                    /// Number of supplied tags
   int enableExifTags;                 /// Enable/Disable EXIF tags in output
   int timelapse;                      /// Delay between each picture in timelapse mode. If 0, disable timelapse
   int fullResPreview;                 /// If set, the camera preview port runs at capture resolution. Reduces fps.
   int frameNextMethod;                /// Which method to use to advance to next frame
   int useGL;                          /// Render preview using OpenGL
   int glCapture;                      /// Save the GL frame-buffer instead of camera output
   int burstCaptureMode;               /// Enable burst mode
   int onlyLuma;                       /// Only output the luma / Y plane of the YUV data
   int datetime;                       /// Use DateTime instead of frame#
   int timestamp;                      /// Use timestamp instead of frame#
   int restart_interval;               /// JPEG restart interval. 0 for none.
   int total_photos;
   int current_photo;

   RASPIPREVIEW_PARAMETERS preview_parameters;    /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   MMAL_COMPONENT_T *encoder_component;   /// Pointer to the encoder component
   // MMAL_COMPONENT_T *null_sink_component; /// Pointer to the null sink component
   // MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview
   MMAL_CONNECTION_T *encoder_connection; /// Pointer to the connection from camera to encoder

   MMAL_POOL_T *encoder_pool; /// Pointer to the pool of buffers used by encoder output port
   MMAL_POOL_T *raw_pool;              /// Pointer to the pool of buffers used by camera stills port

   PORT_STILL_USERDATA callback_data;        /// Used to move data to the encoder callback
   PORT_STILL_USERDATA callback_raw_data;        /// Used to move data to the encoder callback

   int fd;

   // RASPITEX_STATE raspitex_state; /// GL renderer state and parameters
};
