#include "first_app.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "simple_render_system.hpp"
#include "lve_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <chrono>
#include <iostream>
#include <stdexcept>



namespace lve {

    struct GlobalUbo{
        alignas(16) glm::mat4 projectionView{1.f};
        alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
    };

FirstApp::FirstApp() {
    globalPool = LveDescriptorPool::Builder(lveDevice)
    .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .build();
    //you can add multiple addpoolsizes
    //you can store different kinds of type but not the same type. You can set 2 uniform buffer descriptors together

    loadGameObjects();
}

FirstApp::~FirstApp() {
   
}

void FirstApp::run() {

    std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for(auto &uboBuffer:uboBuffers ){
        uboBuffer = std::make_unique<LveBuffer>(
            lveDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffer->map();
    }

    auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
    .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i<globalDescriptorSets.size(); i++){
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        LveDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
    }
    
    SimpleRenderSystem simpleRendereSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
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
            int frameIndex = lveRenderer.getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex]
            };

            //update
            GlobalUbo ubo{};
            ubo.projectionView = camera.getProjection() * camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            //render
            lveRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRendereSystem.renderGameObjects(frameInfo, gameObjects);
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
