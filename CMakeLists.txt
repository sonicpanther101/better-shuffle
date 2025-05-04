cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(better-shuffle)

macro(print_all_variables)
message(STATUS "print_all_variables------------------------------------------{")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
message(STATUS "print_all_variables------------------------------------------}")
endmacro()

# Add the libsndfile cmake module

file(GLOB SOURCES "src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

# Link the libraries
target_link_libraries(${PROJECT_NAME})

# Link the headers
target_include_directories(${PROJECT_NAME} PUBLIC )


# print_all_variables()
