
# raspistill/raspivid/raspiyuv

SET(COMPILE_DEFINITIONS -Werror)

# Set --no-as-needed to stop the linker discarding mmal_vc_client
# as it can't see that the constructor registers a load of functionality
# with the MMAL core.
SET( CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed" )
SET( CMAKE_CXX_STANDARD 11 )

include_directories(${PROJECT_SOURCE_DIR}/host_applications/linux/libs/bcm_host/include)
include_directories(${PROJECT_SOURCE_DIR}/host_applications/linux/apps/raspicam/)
include_directories(${PROJECT_SOURCE_DIR}/host_applications/linux/libs/sm)
include_directories(${PROJECT_SOURCE_DIR}/jsmn)
include_directories(${PROJECT_SOURCE_DIR}/opencv/modules/core/include)
include_directories(${PROJECT_SOURCE_DIR}/opencv/modules/imgproc/include)

# Find the commit hash of the build and pass to the compiler
execute_process(
  # COMMAND git log -1 --abbrev=12 --format=%h
  COMMAND bash "-c" "git log -1 --abbrev=12 --format=%h"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_ID
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions("-DGIT_COMMIT_ID=\"${GIT_COMMIT_ID}\"")

# Determine if we are tainted
execute_process(
  COMMAND bash "-c" "git ls-files -m | wc -l"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_TAINTED
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions("-DGIT_TAINTED=${GIT_TAINTED}")

set (COMMON_SOURCES
   RaspiCamControl.c
   RaspiCamControlStub.c
   RaspiCmdCustom.c
   RaspiCLI.c
   RaspiPreview.c
   RaspiCommonSettings.c
   RaspiHelpers.c
   RaspiGPS.c
   RaspiJson.c
   libgps_loader.c)

if(NOT ARM64)
  set (EGL_LIBS brcmGLESv2 brcmEGL)
  set (EGL_SOURCES RaspiTex.c RaspiTexUtil.c tga.c)
  set (GL_SCENE_SOURCES
     gl_scenes/models.c
     gl_scenes/mirror.c
     gl_scenes/yuv.c
     gl_scenes/sobel.c
     gl_scenes/square.c
     gl_scenes/teapot.c
     gl_scenes/vcsm_square.c)
else()
  set (EGL_SOURCES RaspiTexStub.c)
endif()

add_executable(raspistill ${COMMON_SOURCES} RaspiStill.c  ${EGL_SOURCES} ${GL_SCENE_SOURCES} )
add_executable(raspiyuv   ${COMMON_SOURCES} RaspiStillYUV.c)
add_executable(raspivid   ${COMMON_SOURCES} RaspiVid.c)
add_executable(raspividyuv  ${COMMON_SOURCES} RaspiVidYUV.c)
add_executable(raspi360   ${COMMON_SOURCES} Raspi360.c)

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)
target_link_libraries(raspistill ${MMAL_LIBS} vcos bcm_host ${EGL_LIBS} m dl)
target_link_libraries(raspiyuv   ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(raspivid   ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(raspividyuv   ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(raspi360   ${MMAL_LIBS} vcos bcm_host)

add_library(opencv_core SHARED IMPORTED)
add_library(opencv_imgproc SHARED IMPORTED)
set_target_properties(opencv_core PROPERTIES IMPORTED_LOCATION build_cv/arm_linux/release/lib/libopencv_core.so LINKER_LANGUAGE CXX)
set_target_properties(opencv_imgproc PROPERTIES IMPORTED_LOCATION build_cv/arm_linux/release/lib/libopencv_imgproc.so LINKER_LANGUAGE CXX)
target_link_libraries(raspi360 opencv_imgproc)

install(TARGETS raspistill raspiyuv raspivid raspividyuv raspi360 RUNTIME DESTINATION bin)
