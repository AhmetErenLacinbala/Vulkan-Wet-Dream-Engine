#include "first_app.hpp"

namespace lve {

    void FirstApp::run(){
        while(!lveWindow.shouldClose()){
            //poll events checks if any events are triggered (like keyboard or mouse input)
            //or dismissed the window etc.
            glfwPollEvents();
        }
    }
}

