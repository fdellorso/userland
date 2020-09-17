#include "RaspiCamControlStub.h"
#include "RaspiCmdCustom.h"

typedef int (*camera_func_t)(MMAL_COMPONENT_T *camera, int parameter);
typedef int (*camera_func2_t)(void *data);

static struct
{
    camera_func_t function;
    int val_min;
    int val_max;
} setting_vector_1[] = {
    /* FUNCTION         */
    /* 0  Sharpness     */ {raspicamcontrol_set_sharpness, -100, 100},
    /* 1  Contrast      */ {raspicamcontrol_set_contrast, -100, 100},
    /* 2  Brightness    */ {raspicamcontrol_set_brightness, 0, 100},
    /* 3  Saturation    */ {raspicamcontrol_set_saturation, -100, 100},
    /* 4  Iso           */ {raspicamcontrol_set_ISO, 100, 800},
    /* 5  Vstab         */ {raspicamcontrol_set_video_stabilisation, 0, 1},
    /* 6  Ev            */ {raspicamcontrol_set_exposure_compensation, -10, 10},
    /* 7  Exposure      */ {raspicamcontrol_set_exposure_mode_stub, 0, 13},
    /* 8  Flicker       */ {raspicamcontrol_set_flicker_avoid_mode_stub, 0, 4},
    /* 9  Awb           */ {raspicamcontrol_set_awb_mode_stub, 0, 11},
    /* 10 Metering      */ {raspicamcontrol_set_metering_mode_stub, 0, 4},
    /* 11 Rotation      */ {raspicamcontrol_set_rotation, 0, 359},
    /* 12 Roi           */ {raspicamcontrol_set_ROI_stub, 0, 4},
    /* 13 Shutter       */ {raspicamcontrol_set_shutter_speed, 0, 6000},
    /* 14 Focus         */ {0, 0, 1025},
    /* 15 Isup          */ {0, 0, 1}};

camera_func2_t setting_vector_2[] = {
    /* FUNCTION         */
    /* 0  Record Video  */ raspicmdcustom_record_video,
    /* 1  Stop Video    */ raspicmdcustom_stop_video,
    /* 2  Take Picture  */ 0,
    /* 3  Available     */ 0,
    /* 4  Available     */ 0,
    /* 5  Available     */ 0,
    /* 6  Available     */ 0,
    /* 7  Available     */ 0,
    /* 8  Available     */ 0,
    /* 9  Available     */ 0,
    /* 10 Available     */ 0,
    /* 11 Available     */ 0,
    /* 12 Available     */ 0,
    /* 13 Available     */ 0,
    /* 14 Available     */ 0,
    /* 15 Available     */ 0};
