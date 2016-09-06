# Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
# Use of this source is governed by the Apache License, Version 2.0 that
# can be found in the LICENSE file, which must be distributed with this
# software.

cmake_minimum_required (VERSION 3.0)
project (twitchsw)

option (TSW_BUILD_TESTS "Enable building unittests" ON)

include (FindCURL)

set (twitchsw_LIBS)
set (twitchsw_INCLUDES)

list (APPEND twitchsw_INCLUDES "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/include")

# Find libobs
find_package (libobs REQUIRED COMPONENTS libobs)
if (libobs_FOUND)
    list (APPEND twitchsw_LIBS libobs)
    list (APPEND twitchsw_INCLUDES ${libobs_INCLUDE_DIRS})
else (libobs_FOUND)
    message (FATAL_ERROR "Aborting.")
endif (libobs_FOUND)

# Find libcurl
if (CURL_FOUND)
    list (APPEND twitchsw_LIBS ${CURL_LIBRARY})
    list (APPEND twitchsw_INCLUDES ${CURL_INCLUDE_DIR})
else (CURL_FOUND)
    message (FATAL_ERROR "Aborting.")
endif (CURL_FOUND)

include_directories (${twitchsw_INCLUDES})

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (BITNESS 32)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (BITNESS 64)
endif ()

if (CMAKE_CL_64)
  enable_language (ASM_MASM)
endif (CMAKE_CL_64)

set (RELEASE_DIR "${CMAKE_BINARY_DIR}/out-Release")
set (DEBUG_DIR "${CMAKE_BINARY_DIR}/out-Debug")
if (WIN32)
set (PLUGINS_DIR "obs-plugins/${BITNESS}bit/")
set (DATA_DIR "data/obs-plugins/${PROJECT_NAME}/")
else (WIN32)
# Mac builds (and presumably unix) don't have the 32bit / 64bit directory structure
set (PLUGINS_DIR "obs-plugins/")
set (DATA_DIR "data/obs-plugins/")
endif (WIN32)

# rapidjson https://github.com/miloyip/rapidjson
set (RAPIDJSON_BUILD_DOC FALSE)
set (RAPIDJSON_BUILD_TESTS FALSE)
set (RAPIDJSON_BUILD_EXAMPLES FALSE)
add_subdirectory (third_party/rapidjson EXCLUDE_FROM_ALL)
include_directories (third_party/rapidjson/include)

add_definitions (-DRAPIDJSON_HAS_STDSTRING)

set (twitchsw_HEADERS
    include/twitchsw/twitchsw.h
    include/twitchsw/compiler.h
    include/twitchsw/http.h
    include/twitchsw/map.h
    include/twitchsw/never-destroyed.h
    include/twitchsw/refs.h
    include/twitchsw/sceneitem.h
    include/twitchsw/scenewatcher.h
    include/twitchsw/string.h
    include/twitchsw/webview.h
    include/twitchsw/workerthread.h)

set (twitchsw_SOURCES
    src/twitchsw.cpp
    src/http.cpp
    src/macros-impl.h
    src/sceneitem.cpp
    src/scenewatcher-impl.h
    src/scenewatcher.cpp
    src/string.cpp
    src/string-impl.h
    src/string-impl.cpp
    src/webview.cpp
    src/workerthread-impl.h
    src/workerthread.cpp
    .editorconfig)

if (WIN32)
add_definitions (-DTSW_WIN32)

# IInternetExplorer ActiveX implementation of WebViewImpl
list (APPEND twitchsw_SOURCES
     src/windows/webview-windows.cpp
     src/windows/webview-windows.h
     src/windows/CDispatch.cpp
     src/windows/CDispatch.h
     src/windows/CDocHostShowUI.cpp
     src/windows/CDocHostShowUI.h
     src/windows/CDocHostUIHandler.cpp
     src/windows/CDocHostUIHandler.h
     src/windows/COleClientSite.cpp
     src/windows/COleClientSite.h
     src/windows/COleInPlaceFrame.cpp
     src/windows/COleInPlaceFrame.h
     src/windows/COleInPlaceSite.cpp
     src/windows/COleInPlaceSite.h
     src/windows/COM.h)
add_definitions (-DTSW_WEBVIEW_WIN32)
endif (WIN32)

if (APPLE)
add_definitions (-DTSW_MAC)

list (APPEND twitchsw_SOURCES
      src/mac/WebViewController.h
      src/mac/WebViewController.mm
      src/mac/webview-wkwebview.h
      src/mac/webview-wkwebview.mm)

find_library(APPKIT_LIBRARY AppKit)
find_library(WEBKIT_LIBRARY WebKit)

list (APPEND twitchsw_LIBS ${APPKIT_LIBRARY} ${WEBKIT_LIBRARY})

add_definitions (-DTSW_WEBVIEW_WKWEBVIEW)
endif (APPLE)

set (twitchsw_DATA
    locale/en-US.ini)

#TODO (caitp): Find a way to only run the command for the current build directory,
#             and not both Debug and Release directories.
foreach (data_file ${twitchsw_DATA})
  add_custom_command (OUTPUT "${RELEASE_DIR}/${DATA_DIR}/${data_file}"
                     COMMAND cmake -E copy
                     "${CMAKE_SOURCE_DIR}/data/${data_file}"
                     "${RELEASE_DIR}/${DATA_DIR}/${data_file}")
  add_custom_command (OUTPUT "${DEBUG_DIR}/${DATA_DIR}/${data_file}"
                     COMMAND cmake -E copy
                     "${CMAKE_SOURCE_DIR}/data/${data_file}"
                     "${DEBUG_DIR}/${DATA_DIR}/${data_file}")
  list (APPEND twitchsw_DATA_DEST "${RELEASE_DIR}/${DATA_DIR}/${data_file}")
  list (APPEND twitchsw_DATA_DEST "${DEBUG_DIR}/${DATA_DIR}/${data_file}")
endforeach ()

add_custom_target (copy-data ALL DEPENDS ${twitchsw_DATA_DEST})


add_library (twitchsw MODULE
            ${twitchsw_SOURCES}
            ${twitchsw_HEADERS})
target_link_libraries (twitchsw ${twitchsw_LIBS})
add_dependencies (twitchsw copy-data)

if (APPLE)
target_compile_options(twitchsw PRIVATE "-xobjective-c++")
target_compile_options(twitchsw PUBLIC "-fobjc-arc")
endif (APPLE)

set_target_properties (twitchsw
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_DIR}/${PLUGINS_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${DEBUG_DIR}/${PLUGINS_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${RELEASE_DIR}/${PLUGINS_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${DEBUG_DIR}/${PLUGINS_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${RELEASE_DIR}/${PLUGINS_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${DEBUG_DIR}/${PLUGINS_DIR}"
    PREFIX "")

add_custom_command (TARGET twitchsw POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory data "${DEBUG_DIR}/${DATA_DIR}")

if (TSW_BUILD_TESTS)
  set (BUILD_GTEST ON)
  set (BUILD_GMOCK OFF)
  if (WIN32)
    set (gtest_disable_pthreads ON)
    set (gtest_force_shared_crt ON)
  endif (WIN32)
  add_subdirectory (third_party/gtest EXCLUDE_FROM_ALL)

  enable_testing ()
  add_subdirectory (test)
endif (TSW_BUILD_TESTS)
