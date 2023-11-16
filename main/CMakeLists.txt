idf_build_get_property(target IDF_TARGET)

idf_component_register(
SRCS "main.cpp"
SRCS "ap/ap.cpp"
SRCS "ap/ap_camera.cpp"

SRCS "bsp/bsp.c"
SRCS "bsp/FreeRTOS/esp_port.c"
SRCS "common/core/qbuffer.c"
SRCS "hw/hw.c"
SRCS "hw/driver/led.c"
SRCS "hw/driver/button.c"
SRCS "hw/driver/cli.c"
SRCS "hw/driver/uart.c"
SRCS "hw/driver/ir_remote.c"
SRCS "hw/driver/camera.c"

SRCS "ap/wifi_ap.c"
SRCS "ap/wifi.c"
SRCS "ap/ap_spiffs.c"
SRCS "ap/file_server.c"
SRCS "ap/ap_eeprom.c"
SRCS "ap/ap_ir_remote.c"


SRCS "lib/esp32-camera/driver/esp_camera.c"
SRCS "lib/esp32-camera/driver/cam_hal.c"
SRCS "lib/esp32-camera/driver/sccb.c"
SRCS "lib/esp32-camera/driver/sensor.c"
SRCS "lib/esp32-camera/sensors/ov2640.c"
#SRCS "lib/esp32-camera/sensors/ov3660.c"
#SRCS "lib/esp32-camera/sensors/ov5640.c"
#SRCS "lib/esp32-camera/sensors/ov7725.c"
#SRCS "lib/esp32-camera/sensors/ov7670.c"
#SRCS "lib/esp32-camera/sensors/nt99141.c"
#SRCS "lib/esp32-camera/sensors/gc0308.c"
#SRCS "lib/esp32-camera/sensors/gc2145.c"
#SRCS "lib/esp32-camera/sensors/gc032a.c"
SRCS "lib/esp32-camera/conversions/yuv.c"
SRCS "lib/esp32-camera/conversions/to_jpg.cpp"
SRCS "lib/esp32-camera/conversions/to_bmp.c"
SRCS "lib/esp32-camera/conversions/jpge.cpp"
SRCS "lib/esp32-camera/conversions/esp_jpg_decode.c"

SRCS "lib/esp32-camera/target/xclk.c"
SRCS "lib/esp32-camera/target/esp32s3/ll_cam.c"

SRCS "lib/fb_gfx/fb_gfx.c"
#SRCS "lib/modules/ai/who_ai_utils.cpp"
#SRCS "lib/modules/ai/who_cat_face_detection.cpp"
#SRCS "lib/modules/ai/who_color_detection.cpp"
#SRCS "lib/modules/ai/who_human_face_detection.cpp"
#SRCS "lib/modules/ai/who_human_face_recognition.cpp"
#SRCS "lib/modules/ai/who_motion_detection.cpp"


INCLUDE_DIRS ""
INCLUDE_DIRS "ap"
INCLUDE_DIRS "bsp"
INCLUDE_DIRS "bsp/FreeRTOS"
INCLUDE_DIRS "common"
INCLUDE_DIRS "common/core"
INCLUDE_DIRS "common/hw/include"
INCLUDE_DIRS "hw"
INCLUDE_DIRS "lib"
INCLUDE_DIRS "lib/esp32-camera/driver/include"
INCLUDE_DIRS "lib/esp32-camera/conversions/include"
INCLUDE_DIRS "lib/esp32-camera/driver/private_include"
INCLUDE_DIRS "lib/esp32-camera/sensors/private_include"
INCLUDE_DIRS "lib/esp32-camera/conversions/private_include"
INCLUDE_DIRS "lib/esp32-camera/target/private_include"

INCLUDE_DIRS "lib/esp-dl/include"
INCLUDE_DIRS "lib/esp-dl/include/detect"
INCLUDE_DIRS "lib/esp-dl/include/image"
INCLUDE_DIRS "lib/esp-dl/include/layer"
INCLUDE_DIRS "lib/esp-dl/include/math"
INCLUDE_DIRS "lib/esp-dl/include/model_zoo"
INCLUDE_DIRS "lib/esp-dl/include/nn"
INCLUDE_DIRS "lib/esp-dl/include/tool"
INCLUDE_DIRS "lib/esp-dl/include/typedef"

INCLUDE_DIRS "lib/esp-sr/include/esp32s3"
INCLUDE_DIRS "lib/fb_gfx/include"

INCLUDE_DIRS "lib/esp-qr"

INCLUDE_DIRS "../managed_components/liux-pro__apriltag"
INCLUDE_DIRS "../managed_components/liux-pro__apriltag/apriltag"
INCLUDE_DIRS "../managed_components/liux-pro__apriltag/apriltag/common"


#INCLUDE_DIRS "lib/modules/ai"


EMBED_FILES "../html/favicon.ico" "../html/upload_script.html"
)

message(STATUS ${IDF_TARGET})
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})

#set(lib     libhuman_face_detect.a
#            libdl.a
#            libcolor_detect.a
#            libmfn.a)
#
#if(${IDF_TARGET} STREQUAL "esp32")
#    set(links   "-L ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/esp32")
#
#elseif(${IDF_TARGET} STREQUAL "esp32s2")
#    set(links   "-L ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/esp32s2")
#
#elseif(${IDF_TARGET} STREQUAL "esp32s3")
#    set(links   "-L ${CMAKE_CURRENT_SOURCE_DIR}/lib/esp-dl/lib/esp32s3")
#
#elseif(${IDF_TARGET} STREQUAL "esp32c3")    
#    set(links   "-L ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/esp32c3")
#
#endif()
#
#
#target_link_libraries(${COMPONENT_TARGET} ${links})
#target_link_libraries(${COMPONENT_TARGET} ${lib})


spiffs_create_partition_image(spiffs ../html FLASH_IN_PROJECT)