cmake_minimum_required(VERSION 3.10)
project(VulkanTest)

set(CMAKE_CXX_STANDARD 17)

# Look for Vulkan
find_package(Vulkan REQUIRED)

# Look for GLFW
find_package(glfw3 REQUIRED)

# Add executable
add_executable(VulkanTest main.cpp)

# Link against Vulkan and GLFW
target_link_libraries(VulkanTest Vulkan::Vulkan glfw)

# Include GLM
target_include_directories(VulkanTest PRIVATE $ENV{CMAKE_INCLUDE_PATH})
