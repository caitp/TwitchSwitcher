# Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
# Use of this source is governed by the Apache License, Version 2.0 that
# can be found in the LICENSE file, which must be distributed with this
# software.

set(macro_unittests_SOURCES
    macro_unittests.cpp
    "${CMAKE_SOURCE_DIR}/src/macros-impl.h")

add_executable(macro_unittests ${macro_unittests_SOURCES})

target_include_directories(macro_unittests PRIVATE
                           ${CMAKE_SOURCE_DIR}/src
                           ${CMAKE_SOURCE_DIR}/include
                           ${gtest_SOURCE_DIR}/include
                           ${gtest_SOURCE_DIR})

target_link_libraries(macro_unittests
                      gtest gtest_main)
