cmake_minimum_required(VERSION 3.27)
project(frameminmax C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
set(CMAKE_OSX_ARCHITECTURES arm64)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)
set(CMAKE_C_FLAGS_RELEASE -Ofast)

include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
)

file(GLOB PROJECT_SRC
	"*.h"
	"*.c"
)

add_library(${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
)

target_compile_options(${PROJECT_NAME} PUBLIC -Wall)
# target_compile_options(${PROJECT_NAME} PUBLIC -Wextra)
target_compile_options(${PROJECT_NAME} PUBLIC -Wpedantic)
# target_compile_options(${PROJECT_NAME} PUBLIC -Wno-unused-variable)
# target_compile_options(${PROJECT_NAME} PUBLIC -Werror)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
