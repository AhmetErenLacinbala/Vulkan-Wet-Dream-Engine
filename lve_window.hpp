#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace lve {
    class LveWindow
    {
        public:
        LveWindow(int w, int h, std::string name);
        LveWindow(const LveWindow&) = delete;
        LveWindow& operator=(const LveWindow&) = delete;
        ~LveWindow();
        bool shouldClose() { return glfwWindowShouldClose(window); }
        bool wasWindowResized() {return framebufferResized;}
        void resetWindowResizeFlag(){framebufferResized = false;}
        VkExtent2D getExtend(){
            return {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
        }

        GLFWwindow* getGLFWwindow() const {return window;}

        void setWindowTitle(std::string title){
            glfwSetWindowTitle(window, title.c_str());
        };

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
        private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
            void initWindow();
            int width;
            int height;
            bool framebufferResized = false;
            std::string windowName;
            GLFWwindow* window;
    };
    
}