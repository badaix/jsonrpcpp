cmake_minimum_required(VERSION 2.8)

project(jsonrpcpp CXX)
set(PROJECT_DESCRIPTION "C++ JSON-RPC 2.0 library")
set(PROJECT_URL "https://github.com/badaix/jsonrpcpp")
set(PROJECT_VERSION "1.0.1")

option(BUILD_SHARED_LIBS "Build jsonrpcpp as a shared library" ON)
option(BUILD_STATIC_LIBS "Build jsonrpcpp as a static library" ON)
option(BUILD_TESTS "Build tests (run tests with make test)" ON)

if (NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
    message(FATAL_ERROR "One or both of BUILD_SHARED_LIBS or BUILD_STATIC_LIBS"
                        "must be set to ON to build")
endif()

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    SET(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
endif()

if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
    SET(CMAKE_INSTALL_INCLUDEDIR include CACHE
        PATH "Output directory for header files")
endif()

include_directories(lib lib/externals)
set(JSONRPCPP_SOURCES lib/jsonrp.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

if (BUILD_SHARED_LIBS)
    add_library(jsonrpcpp SHARED "${JSONRPCPP_SOURCES}")
	if(WIN32)
		install(TARGETS jsonrpcpp RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}")
	else()
		install(TARGETS jsonrpcpp LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
	endif()
    
endif (BUILD_SHARED_LIBS)

if (BUILD_STATIC_LIBS)
    add_library(jsonrpcpp-static STATIC "${JSONRPCPP_SOURCES}")
    set_target_properties(jsonrpcpp-static PROPERTIES OUTPUT_NAME jsonrpcpp)
    install(TARGETS jsonrpcpp-static ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif (BUILD_STATIC_LIBS)

if (BUILD_TESTS)
    if (NOT BUILD_STATIC_LIBS)
        message(FATAL_ERROR "Tests can only be built against static libraries "
                            "(set BUILD_STATIC_LIBS=ON)")
    endif (NOT BUILD_STATIC_LIBS)
    add_executable(jsonrpctest jsonrpctest.cpp)
    target_link_libraries(jsonrpctest jsonrpcpp-static)
endif (BUILD_TESTS)

install(FILES lib/jsonrp.hpp lib/externals/json.hpp
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jsonrpcpp")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/jsonrpcpp.pc.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/jsonrpcpp.pc"
    @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/jsonrpcpp.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
