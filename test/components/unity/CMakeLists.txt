cmake_minimum_required(VERSION 3.16)

# Unity testing framework component for ESP-IDF
# This component wraps the Unity testing framework for use in ESP-IDF projects

# Unity source files (assuming Unity is available in ESP-IDF or as submodule)
set(UNITY_SOURCES
    "unity.c"
    "unity_test_runner.c"
)

# Check if Unity is available in ESP-IDF components
if(EXISTS "$ENV{IDF_PATH}/components/unity")
    set(UNITY_INCLUDE_DIRS
        "$ENV{IDF_PATH}/components/unity/include"
        "$ENV{IDF_PATH}/components/unity/unity/src"
    )
    set(UNITY_SOURCES
        "$ENV{IDF_PATH}/components/unity/unity/src/unity.c"
    )
else()
    # Use local Unity if available
    set(UNITY_INCLUDE_DIRS
        "include"
        "unity/src"
    )
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/unity/src/unity.c")
        set(UNITY_SOURCES
            "unity/src/unity.c"
        )
    else()
        message(WARNING "Unity not found. Please install Unity testing framework.")
        set(UNITY_SOURCES
            "unity_stub.c"  # Fallback stub
        )
    endif()
endif()

# Register Unity component
idf_component_register(
    SRCS ${UNITY_SOURCES}
    INCLUDE_DIRS ${UNITY_INCLUDE_DIRS}
    REQUIRES esp_common log
)

# Set Unity configuration
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    UNITY_INCLUDE_CONFIG_H=1
    UNITY_SUPPORT_64=1
    UNITY_INCLUDE_DOUBLE=1
    UNITY_INCLUDE_FLOAT=1
    UNITY_EXCLUDE_SETJMP_H=1
    UNITY_OUTPUT_COLOR=1
)

# Add Unity configuration options
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-format
    -Wno-unused-function
)