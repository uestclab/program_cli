cmake_minimum_required(VERSION 3.5)
option(PUBLIC "This is a default option for PUBLIC" OFF)
message(STATUS "build public platform switch:"${PUBLIC})
project(program_cli)

include_directories("/home/dpdk/liqing/code/GW_handover/handover_Server/dependence/include/")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/lib")

if(${PUBLIC} STREQUAL "ON")
	message(STATUS "start to build ARM platform.")
	# cross complie
	SET(CMAKE_SYSTEM_NAME Linux)
	SET(CMAKE_SYSTEM_PROCESSOR arm) 
	SET(TOOLCHAIN_DIR "/home/dpdk/liqing/petalinux/gcc-arm-linux-gnueabi")
	SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
	SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

	SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc-6.2.1)
	SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)
	link_directories("/home/dpdk/liqing/code/GW_handover/handover_Server/dependence/lib_arm/")
	# zlog
	link_directories("/home/dpdk/liqing/common/lib/")
	include_directories("/home/dpdk/liqing/common/include/")
	# cross complie

elseif(${PUBLIC} STREQUAL "OFF")
    message(STATUS "start to build PC platform.")
	message(STATUS "PROJECT_SOURCE_DIR": ${PROJECT_SOURCE_DIR})
endif(${PUBLIC} STREQUAL "ON")

SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


SET(CMAKE_VERBOSE_MAKEFILE ON)    
SET(ROOT_PATH ".")

file(GLOB_RECURSE SOURCE_FILES "src/*.c")
file(GLOB_RECURSE ALL_EXECUTABLES "program_cli.c")

add_executable(${PROJECT_NAME} ${ALL_EXECUTABLES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} -pthread)
target_link_libraries(${PROJECT_NAME} -lm)

if(${PUBLIC} STREQUAL "ON") # arm
	#target_link_libraries(${PROJECT_NAME} -lzlog)
	#target_link_libraries(${PROJECT_NAME} cjson)
	#target_link_libraries(${PROJECT_NAME} broker)
	#target_link_libraries(${PROJECT_NAME} cst)
	#target_link_libraries(${PROJECT_NAME} mosquitto)
	#target_link_libraries(${PROJECT_NAME} gwapp)

endif(${PUBLIC} STREQUAL "ON")













