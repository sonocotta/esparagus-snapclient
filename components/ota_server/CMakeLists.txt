idf_component_register(SRCS "ota_server.c"
                       INCLUDE_DIRS "include"
                       REQUIRES app_update dsp_processor wifi_interface lightsnapcast)
