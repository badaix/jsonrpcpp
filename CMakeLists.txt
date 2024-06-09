#    __  ____   __   __ _  ____  ____   ___  _    _   
#  _(  )/ ___) /  \ (  ( \(  _ \(  _ \ / __)( )  ( )  
# / \) \\___ \(  O )/    / )   / ) __/( (__(_ _)(_ _) 
# \____/(____/ \__/ \_)__)(__\_)(__)   \___)(_)  (_)  

# This file is part of jsonrpc++
# Copyright (C) 2017-2021 Johannes Pohl

# This software may be modified and distributed under the terms
# of the MIT license.  See the LICENSE file for details.


cmake_minimum_required(VERSION 3.14)

project(jsonrpcpp VERSION 1.3.2 LANGUAGES CXX)
set(PROJECT_DESCRIPTION "C++ JSON-RPC 2.0 library")
set(PROJECT_URL "https://github.com/badaix/jsonrpcpp")

option(BUILD_EXAMPLE "Build example (build jsonrpcpp_example demo)" ON)
option(BUILD_TESTS "Build tests" ON)
option(WERROR "Treat warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)


if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
	SET(CMAKE_INSTALL_INCLUDEDIR include CACHE
		PATH "Output directory for header files")
endif()

include_directories(
	"include"
)

install(FILES include/jsonrpcpp.hpp include/json.hpp
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jsonrpcpp")

if(MSVC)
    # warning level 4 and all warnings as errors warning C4505: 'getArch':
    # unreferenced local function has been removed warning C4458: declaration of
    # 'size' hides class member warning C4459: declaration of 'query' hides global
    # declaration
    add_compile_options(/W4 /wd4458 /wd4459 /wd4505)
    if(WERROR)
        add_compile_options(/WX)
    endif()
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic)

    if(WERROR)
        add_compile_options(-Werror)
    endif()
endif()
  

if (BUILD_EXAMPLE)
add_subdirectory(example)
endif (BUILD_EXAMPLE)

if (BUILD_TESTS)
add_subdirectory(test)
endif (BUILD_TESTS)

FIND_PROGRAM(CLANG_FORMAT "clang-format")
IF(CLANG_FORMAT)
	set(CHECK_CXX_SOURCE_FILES
	${CMAKE_SOURCE_DIR}/include/jsonrpcpp.hpp
	${CMAKE_SOURCE_DIR}/example/jsonrpcpp_example.cpp
	${CMAKE_SOURCE_DIR}/test/test_main.cpp
	)

    list(REMOVE_ITEM CHECK_CXX_SOURCE_FILES "${CMAKE_SOURCE_DIR}/common/json.hpp")

    ADD_CUSTOM_TARGET(
        reformat
        COMMAND
        ${CLANG_FORMAT}
        -i
        -style=file
        ${CHECK_CXX_SOURCE_FILES}
        COMMENT "Auto formatting of all source files"
    )
ENDIF()


