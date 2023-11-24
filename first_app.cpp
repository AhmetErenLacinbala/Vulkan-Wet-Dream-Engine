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
// temporary helper function, creates a 1x1x1 cube centered at offset with an index buffer
std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
  LveModel::Builder modelBuilder{};
  modelBuilder.vertices = {
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
  };
  for (auto& v : modelBuilder.vertices) {
    v.position += offset;
  }

  modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                          12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

  return std::make_unique<LveModel>(device, modelBuilder);
}

void FirstApp::loadGameObjects() {
    std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, {0.f, 0.f, 0.f});
    auto cube = LveGameObject::createGameObject();
    cube.model = lveModel;
    cube.transform.translation = {0.f, 0.f, 2.5f};
    cube.transform.scale = {.5f, .5f, .5f};
    //reason of this tranform:
    //x from [-1, 1], y from [-1, 1] but z from [0, 1]
    //because the z value is form 0 to 1 front half to he object will be cliped since it is bigger than viewing volume
    //so we move it on z axis then scale it by half

    gameObjects.push_back(std::move(cube));
}

} // namespace lve
