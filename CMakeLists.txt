cmake_minimum_required(VERSION 3.10)
project(VulkanTest)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES 
    main.cpp
    lve_window.cpp
    first_app.cpp
    lve_pipeline.cpp
    lve_device.cpp
    lve_swap_chain.cpp
    lve_model.cpp
    lve_renderer.cpp
    simple_render_system.cpp
    lve_camera.cpp
    keyboard_movement_controller.cpp
    lve_game_object.cpp
    lve_buffer.cpp
    lve_descriptors.cpp
    point_light_system.cpp
    baseTerrain.cpp
)

set(HEADERS
    lve_window.hpp
    first_app.hpp
    lve_pipeline.hpp
    lve_device.hpp
    lve_swap_chain.hpp
    lve_model.hpp
    lve_game_object.hpp
    lve_renderer.hpp
    simple_render_system.hpp
    lve_camera.hpp
    keyboard_movement_controller.hpp
    tiny_obj_loader.h
    lve_utils.hpp
    lve_buffer.hpp
    lve_frame_info.hpp
    lve_descriptors.hpp
    point_light_system.hpp
    baseTerrain.hpp
)

# Find Vulkan, GLFW, and GLM
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLM REQUIRED)

# Create the executable
add_executable(VulkanTest ${SOURCES} ${HEADERS})

# Include directories for Vulkan, GLFW, and GLM
target_include_directories(VulkanTest PRIVATE ${Vulkan_INCLUDE_DIRS})
target_include_directories(VulkanTest PRIVATE ${GLM_INCLUDE_DIRS})

# Link against Vulkan and GLFW
target_link_libraries(VulkanTest Vulkan::Vulkan glfw)

# Copy shader files to build directory
add_custom_command(
    TARGET VulkanTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:VulkanTest>/shaders
)

# Copy model files to build directory
add_custom_command(
    TARGET VulkanTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/models
        $<TARGET_FILE_DIR:VulkanTest>/models
)

add_custom_command(
    TARGET VulkanTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:VulkanTest>/data
)

# Run compile.sh script
add_custom_command(
    TARGET VulkanTest POST_BUILD
    COMMAND chmod +x ${CMAKE_SOURCE_DIR}/compile.sh  # Make the script executable
    COMMAND ${CMAKE_SOURCE_DIR}/compile.sh
)
