#include "first_app.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "simple_render_system.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <chrono>
#include <iostream>
#include <stdexcept>



namespace lve {

FirstApp::FirstApp() {
    loadGameObjects();
}

FirstApp::~FirstApp() {
   
}

void FirstApp::run() {
    SimpleRenderSystem simpleRendereSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
    LveCamera camera {};


    auto viewerObject = LveGameObject::createGameObject();
    KeyboardMovementController cameraController{};

    std::cout << "max push conts size = " << lveDevice.properties.limits.maxPushConstantsSize << "\n";


    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!lveWindow.shouldClose()) {
        // poll events checks if any events are triggered (like keyboard or mouse input)
        // or dismissed the window etc.
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime-currentTime).count();
        currentTime = newTime;

        frameTime = glm::min(frameTime, 0.1f);

        cameraController.moveInPlanceXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = lveRenderer.getAspectRatio();
        //camera.setOrthographicProjection(-aspect,aspect,-1,1,-1,1);
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
        if (auto commandBuffer = lveRenderer.beginFrame()){
            lveRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRendereSystem.renderGameObjects(commandBuffer, gameObjects, camera);
            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
        }

        vkDeviceWaitIdle(lveDevice.device());
    }
}

void FirstApp::loadGameObjects() {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "./models/smooth_vase.obj");
    auto gameObject = LveGameObject::createGameObject();
    gameObject.model = lveModel;
    gameObject.transform.translation = {0.f, .5f, 2.5f};
    gameObject.transform.scale = {2.f, 2.f, 2.f};
    //reason of this tranform:
    //x from [-1, 1], y from [-1, 1] but z from [0, 1]
    //because the z value is form 0 to 1 front half to he object will be cliped since it is bigger than viewing volume
    //so we move it on z axis then scale it by half

    gameObjects.push_back(std::move(gameObject));
}

} // namespace lve
