#include "RaspiCamControlStub.h"


int raspicamcontrol_set_exposure_mode_stub(MMAL_COMPONENT_T *camera, int parameter)
{
    return raspicamcontrol_set_exposure_mode(camera, (MMAL_PARAM_EXPOSUREMODE_T) parameter);
}


int raspicamcontrol_set_flicker_avoid_mode_stub(MMAL_COMPONENT_T *camera, int parameter)
{
    return raspicamcontrol_set_flicker_avoid_mode(camera, (MMAL_PARAM_FLICKERAVOID_T) parameter);
}


int raspicamcontrol_set_awb_mode_stub(MMAL_COMPONENT_T *camera, int parameter)
{
    return raspicamcontrol_set_awb_mode(camera, (MMAL_PARAM_AWBMODE_T) parameter);
}


int raspicamcontrol_set_metering_mode_stub(MMAL_COMPONENT_T *camera, int parameter)
{
    return raspicamcontrol_set_metering_mode(camera, (MMAL_PARAM_EXPOSUREMETERINGMODE_T) parameter);
}


int raspicamcontrol_set_ROI_stub(MMAL_COMPONENT_T *camera, int parameter)
{
    PARAM_FLOAT_RECT_T rect;

    rect.x = 0.5;
    rect.y = 0.5;
    rect.w = 1 / parameter;
    rect.h = 1 / parameter;

    return raspicamcontrol_set_ROI(camera, rect);
}