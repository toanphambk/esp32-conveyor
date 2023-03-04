idf_component_register(SRCS "main.c"
                            "gate_driver.c" 
                            "io_driver.c"
                            "encoder.c"
                            "tcp_client.c"
                            "wifi_station.c"
                            "process_data.c"
                        INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
