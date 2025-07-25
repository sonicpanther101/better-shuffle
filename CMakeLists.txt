cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(_SILENCE_CXX17_ALLOCATOR_MEMBERS_DEPRECATION_WARNING)
add_compile_definitions(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

project(better-shuffle)

macro(print_all_variables)
message(STATUS "print_all_variables------------------------------------------{")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
message(STATUS "print_all_variables------------------------------------------}")
endmacro()

# Tiny DNN
add_subdirectory(vendor/tiny_dnn)


include_directories(
    vendor/tiny_dnn
    vendor/tiny_dnn/tiny_dnn
    vendor/tiny_dnn/cereal/include
    src
)

file(GLOB SOURCES "src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} tiny_dnn)

# print_all_variables()