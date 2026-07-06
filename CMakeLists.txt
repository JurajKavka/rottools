# Set the minimum required version of CMake
cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the project name
project(rotreader)

# Export compile commands for VSCode IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find the wxWidgets library and its components
find_package(wxWidgets COMPONENTS core base stc webview REQUIRED)

# Include the wxWidgets CMake configuration
include(${wxWidgets_USE_FILE})

add_subdirectory(HelperFunctions)
add_subdirectory(Backend)
add_subdirectory(Components)

# Add the executable target from our source file
add_executable(rotreader main.cpp)

target_link_libraries(rotreader PRIVATE ${wxWidgets_LIBRARIES} Backend Components)
target_compile_features(rotreader PRIVATE cxx_std_20)
