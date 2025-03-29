# project configuration
cmake_minimum_required(VERSION 3.10.0)
project(lea VERSION 0.1.0 LANGUAGES C CXX)

option(CMAKE_VERBOSE_LOGGING "Enable detailed logging of targets" ON)
option(BUILD_TESTING "Enable tests" ON)

# default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif()


# prints basic info for a target when CMAKE_VERBOSE_LOGGING ON
function(log_target_info TARGET_NAME)
	if (CMAKE_VERBOSE_LOGGING)
		message(STATUS "----- Target: ${TARGET_NAME} -----")
		get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
		get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
		get_target_property(TARGET_INCLUDE_DIRS ${TARGET_NAME} INCLUDE_DIRECTORIES)
		get_target_property(TARGET_LINK_LIBS ${TARGET_NAME} LINK_LIBRARIES)
		get_target_property(TARGET_COMPILE_OPTIONS ${TARGET_NAME} COMPILE_OPTIONS)

		message(STATUS "Type: ${TARGET_TYPE}")
		message(STATUS "Sources: ${TARGET_SOURCES}")
		message(STATUS "Include Dirs: ${TARGET_INCLUDE_DIRS}")
		message(STATUS "Linked Libraries: ${TARGET_LINK_LIBS}")
		message(STATUS "Compile Options: ${TARGET_COMPILE_OPTIONS}")
		message(STATUS "-----------------------------------")
	endif()
endfunction()

# build settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# set targets and includes
set(TRGT_CORE ${CMAKE_PROJECT_NAME}-core)
set(TRGT_APP ${CMAKE_PROJECT_NAME}-app)

set(CORE_INCLUDES 
    ${CMAKE_SOURCE_DIR}/include/core/
)

set(APP_INCLUDES
    ${CMAKE_SOURCE_DIR}/include/app
)

file(GLOB_RECURSE CORE_SOURCES "${CMAKE_SOURCE_DIR}/src/core/*.cpp")
file(GLOB_RECURSE APP_SOURCES "${CMAKE_SOURCE_DIR}/src/app/*.cpp")

# build gtest files from test/
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

# build core library
add_library(${TRGT_CORE} STATIC ${CORE_SOURCES})
target_include_directories(${TRGT_CORE} PUBLIC ${CORE_INCLUDES})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TRGT_CORE} PRIVATE -O3)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Dev")
    target_compile_options(${TRGT_CORE} PRIVATE -g)
endif()

log_target_info(${TRGT_CORE})

# build app executable
add_executable(${TRGT_APP} ${APP_SOURCES}) 
target_link_libraries(${TRGT_APP} PRIVATE ${TRGT_CORE})
include_directories(${TRGT_APP} PUBLIC ${APP_INCLUDES})

log_target_info(${TRGT_APP})

