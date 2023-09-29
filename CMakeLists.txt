cmake_minimum_required(VERSION 3.10)
project(VulkanTest)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES 
    main.cpp
    lve_window.cpp
    first_app.cpp
    lve_pipeline.cpp
    lve_device.cpp
    )

set(HEADERS
    lve_window.hpp
    first_app.hpp
    lve_pipeline.hpp
    lve_device.hpp
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

# Copy shader files to build directory
add_custom_command(
    TARGET VulkanTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:VulkanTest>/shaders
)
