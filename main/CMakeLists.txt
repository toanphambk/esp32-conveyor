idf_component_register(SRCS "wifi.c" "main.c"
                            "encoder.c"
                            "tcp_client.c"
                           
                        INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
