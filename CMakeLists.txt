cmake_minimum_required(VERSION 3.0)
project(twitchsw)

include(FindCURL)

set(twitchsw_LIBS)
set(twitchsw_INCLUDES)

# Find libobs
find_package(libobs REQUIRED COMPONENTS libobs)
if (libobs_FOUND)
    list(APPEND twitchsw_LIBS libobs)
    list(APPEND twitchsw_INCLUDES ${libobs_INCLUDE_DIRS})
else (libobs_FOUND)
    message(FATAL_ERROR "Aborting.")
endif (libobs_FOUND)

# Find libcurl
if (CURL_FOUND)
    list(APPEND twitchsw_LIBS ${CURL_LIBRARY})
    list(APPEND twitchsw_INCLUDES ${CURL_INCLUDE_DIR})
else (CURL_FOUND)
    message(FATAL_ERROR "Aborting.")
endif (CURL_FOUND)

include_directories(${twitchsw_INCLUDES})

set(BITNESS 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(BITNESS 64)
endif()

set(RELEASE_DIR "${CMAKE_BINARY_DIR}/out-Release")
set(DEBUG_DIR "${CMAKE_BINARY_DIR}/out-Debug")
set(PLUGINS_DIR "obs-plugins/${BITNESS}bit/")
set(DATA_DIR "data/obs-plugins/${PROJECT_NAME}/")

set(twitchsw_HEADERS
    src/twitchsw.h
    src/http.h
    src/refs.h
    src/sceneitem.h
    src/scenewatcher.h
    src/scenewatcher-impl.h
    src/string.h
    src/workerthread.h
    src/workerthread-impl.h)

set(twitchsw_SOURCES
    src/twitchsw.cpp
    src/http.cpp
    src/sceneitem.cpp
    src/scenewatcher.cpp
    src/string.cpp
    src/workerthread.cpp
    .editorconfig)

set(twitchsw_DATA
    locale/en-US.ini)

#TODO(caitp): Find a way to only run the command for the current build directory,
#             and not both Debug and Release directories.
foreach(data_file ${twitchsw_DATA})  
  add_custom_command(OUTPUT "${RELEASE_DIR}/${DATA_DIR}/${data_file}"
                     COMMAND cmake -E copy
                     "${CMAKE_SOURCE_DIR}/data/${data_file}"
                     "${RELEASE_DIR}/${DATA_DIR}/${data_file}")
  add_custom_command(OUTPUT "${DEBUG_DIR}/${DATA_DIR}/${data_file}"
                     COMMAND cmake -E copy
                     "${CMAKE_SOURCE_DIR}/data/${data_file}"
                     "${DEBUG_DIR}/${DATA_DIR}/${data_file}")
  list(APPEND twitchsw_DATA_DEST "${RELEASE_DIR}/${DATA_DIR}/${data_file}")
  list(APPEND twitchsw_DATA_DEST "${DEBUG_DIR}/${DATA_DIR}/${data_file}")
endforeach()

add_custom_target(copy-data ALL DEPENDS ${twitchsw_DATA_DEST})


add_library(twitchsw MODULE
            ${twitchsw_SOURCES}
            ${twitchsw_HEADERS})
target_link_libraries(twitchsw ${twitchsw_LIBS})
add_dependencies(twitchsw copy-data)

set_target_properties(twitchsw
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_DIR}/${PLUGINS_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${DEBUG_DIR}/${PLUGINS_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${RELEASE_DIR}/${PLUGINS_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${DEBUG_DIR}/${PLUGINS_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${RELEASE_DIR}/${PLUGINS_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${DEBUG_DIR}/${PLUGINS_DIR}")

add_custom_command(TARGET twitchsw POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory data "${DEBUG_DIR}/${DATA_DIR}")
