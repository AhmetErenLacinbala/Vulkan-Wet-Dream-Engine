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
)

set(HEADERS
    lve_window.hpp
    first_app.hpp
    lve_pipeline.hpp
    lve_device.hpp
    lve_swap_chain.hpp
    lve_model.hpp
    lve_game_object.hpp
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

# Run compile.sh script
add_custom_command(
    TARGET VulkanTest POST_BUILD
    COMMAND chmod +x ${CMAKE_SOURCE_DIR}/compile.sh  # Make the script executable
    COMMAND ${CMAKE_SOURCE_DIR}/compile.sh
)
