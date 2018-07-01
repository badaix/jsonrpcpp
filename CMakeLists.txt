#    __  ____   __   __ _  ____  ____   ___  _    _   
#  _(  )/ ___) /  \ (  ( \(  _ \(  _ \ / __)( )  ( )  
# / \) \\___ \(  O )/    / )   / ) __/( (__(_ _)(_ _) 
# \____/(____/ \__/ \_)__)(__\_)(__)   \___)(_)  (_)  

# This file is part of jsonrpc++
# Copyright (C) 2017-2018 Johannes Pohl

# This software may be modified and distributed under the terms
# of the MIT license.  See the LICENSE file for details.


cmake_minimum_required(VERSION 3.0.0)

project(jsonrpcpp VERSION 1.2.0 LANGUAGES CXX)
set(PROJECT_DESCRIPTION "C++ JSON-RPC 2.0 library")
set(PROJECT_URL "https://github.com/badaix/jsonrpcpp")

option(BUILD_EXAMPLE "Build example (build jsonrpcpp_example demo)" ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)


if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
	SET(CMAKE_INSTALL_INCLUDEDIR include CACHE
		PATH "Output directory for header files")
endif()


include_directories(
	"include"
)

if (BUILD_EXAMPLE)
	add_executable(jsonrpcpp_example jsonrpcpp_example.cpp)
endif (BUILD_EXAMPLE)


install(FILES include/jsonrpcpp.hpp include/json.hpp
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jsonrpcpp")

