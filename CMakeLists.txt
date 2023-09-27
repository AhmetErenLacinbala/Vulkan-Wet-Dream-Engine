cmake_minimum_required(VERSION 3.10)
project(VulkanTest)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES 
    main.cpp
    lve_window.cpp
    first_app.cpp
    )

set(HEADERS
    lve_window.hpp
    first_app.hpp

)


# Look for Vulkan
find_package(Vulkan REQUIRED)

# Look for GLFW
find_package(glfw3 REQUIRED)

add_executable(VulkanTest ${SOURCES} ${HEADERS})

# Link against Vulkan and GLFW
target_link_libraries(VulkanTest Vulkan::Vulkan glfw)

# Include GLM
target_include_directories(VulkanTest PRIVATE $ENV{CMAKE_INCLUDE_PATH})
