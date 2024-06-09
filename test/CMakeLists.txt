include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

# Make test executable
add_executable(jsonrpcpp_test ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp)

target_link_libraries(jsonrpcpp_test Catch2::Catch2WithMain)
