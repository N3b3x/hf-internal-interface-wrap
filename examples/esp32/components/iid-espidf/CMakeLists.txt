cmake_minimum_required(VERSION 3.23)

# Collect all source files using glob patterns for easier maintenance
file(GLOB_RECURSE MCU_SOURCES "../../../../src/mcu/*.cpp")
file(GLOB_RECURSE THREAD_SAFE_SOURCES "../../../../src/thread_safe/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES "../../../../src/utils/*.cpp")

# Combine all source files
set(ALL_SOURCES
    ${MCU_SOURCES}
    ${THREAD_SAFE_SOURCES}
    ${UTILS_SOURCES}
)

idf_component_register(
    SRCS ${ALL_SOURCES}
        
    INCLUDE_DIRS 
        "../../../../inc"
        "../../../../inc/base"
        "../../../../inc/mcu"
        "../../../../inc/mcu/utils"
        "../../../../inc/thread_safe"
        "../../../../inc/utils"
        
    REQUIRES driver esp_adc freertos esp_timer nvs_flash nvs_sec_provider
)
