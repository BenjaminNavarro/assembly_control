cmake_minimum_required (VERSION 2.6)

project(AssemblyControl)

set(EXECUTABLE_OUTPUT_PATH ../bin)

if(APPLE)
	set(VREP_PATH "/Applications/V-REP/programming")
	set(VREP_CFLAGS "-DNON_MATLAB_PARSING -DMAX_EXT_API_CONNECTIONS=255 -D__APPLE__")
elseif(UNIX)
	set(VREP_PATH "$ENV{HOME}/V-REP/programming")
	set(VREP_CFLAGS "-DNON_MATLAB_PARSING -DMAX_EXT_API_CONNECTIONS=255 -D__linux")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VREP_CFLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${VREP_CFLAGS} -fPIC")

find_package( Threads )

# Library
file(
        GLOB_RECURSE
        lib_source_files
        src/lib/*
        ${VREP_PATH}/remoteApi/*
)

add_library(simulator ${lib_source_files})
target_link_libraries(simulator ${CMAKE_THREAD_LIBS_INIT})

include_directories(src/lib)
include_directories(src/lib/vrep)

# Applications
file(
        GLOB_RECURSE
        application_source_files
        src/application/simulator/*
)

file(
        GLOB_RECURSE
        simple_application_source_files
        src/application/simple_simulator/*
)

add_executable(assembly_control ${application_source_files})
target_link_libraries(assembly_control simulator)

add_executable(simple_assembly_control ${simple_application_source_files})
target_link_libraries(simple_assembly_control simulator)

# Examples
file(
        GLOB_RECURSE
        example_source_files
        src/example/simulator/*
)

file(
        GLOB_RECURSE
        simple_example_source_files
        src/example/simple_simulator/*
)


file(
        GLOB_RECURSE
        tasks_example_source_files
        src/example/two_tasks/*
)


add_executable(example ${example_source_files})
target_link_libraries(example simulator)

add_executable(simple_example ${simple_example_source_files})
target_link_libraries(simple_example simulator)

add_executable(tasks_example ${tasks_example_source_files})
target_link_libraries(tasks_example simulator)

#add_custom_command(TARGET assembly_control
#                   POST_BUILD
#                  COMMAND sh ARGS make.sh
#			   	   WORKING_DIRECTORY ../py)
