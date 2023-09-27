#include "first_app.hpp"


#include <stdexcept>
#include <iostream>
#include <cstdlib>

int main(){
    lve::FirstApp app{};
    try {
        app.run();
        std::cout<<"Hello Vulkan!"<<std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::cout<<"Hello Vulkan!2"<<std::endl;
    }
    return EXIT_SUCCESS;
}