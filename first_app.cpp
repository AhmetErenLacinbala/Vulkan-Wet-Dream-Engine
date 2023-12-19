#include "first_app.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "simple_render_system.hpp"
#include "point_light_system.hpp"
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
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::vec4 ambientLightColor{1.f,1.f,1.f,0.02f}; //w is intensity
        glm::vec3 lightPosition{-1.f};
        alignas(32)glm::vec4 lightColor{1.f}; //w is light intensity
        //vec4 lightColor -> (r,g,b,intensity) -> (rxi, gxi, bxi)
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
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
    .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i<globalDescriptorSets.size(); i++){
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        LveDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
    }
    
    SimpleRenderSystem simpleRendereSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
    PointLightSytem pointLightSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
    LveCamera camera {};


    auto viewerObject = LveGameObject::createGameObject();
    viewerObject.transform.translation.z = -2.5f;
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
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
        if (auto commandBuffer = lveRenderer.beginFrame()){
            int frameIndex = lveRenderer.getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                gameObjects
            };

            //update
            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            //render
            lveRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRendereSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);
            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
        }

        vkDeviceWaitIdle(lveDevice.device());
    }
}

void FirstApp::loadGameObjects() {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "./models/smooth_vase.obj");
    auto gameObject1 = LveGameObject::createGameObject();
    gameObject1.model = lveModel;
    gameObject1.transform.translation = {0.f, .5f, 1.f};
    gameObject1.transform.scale = {2.f, 2.f, 2.f};
    gameObjects.emplace(gameObject1.getId(), std::move(gameObject1));

    auto gameObject2 = LveGameObject::createGameObject();
    gameObject2.model = lveModel;
    gameObject2.transform.translation = {0.f, .5f, 0.f};
    gameObject2.transform.scale = {2.f, 2.f, 2.f};
    gameObjects.emplace(gameObject2.getId(), std::move(gameObject2));
    //reason of this tranform:
    //x from [-1, 1], y from [-1, 1] but z from [0, 1]
    //because the z value is form 0 to 1 front half to he object will be cliped since it is bigger than viewing volume
    //so we move it on z axis then scale it by half

    std::shared_ptr<LveModel> quadModel = LveModel::createModelFromFile(lveDevice, "./models/quad.obj");
    auto quad_floor = LveGameObject::createGameObject();
    quad_floor.model = quadModel;
    quad_floor.transform.translation = {0.f, .5f, 0.f};
    quad_floor.transform.scale = {3.f, 1.f, 3.f};
    gameObjects.emplace(quad_floor.getId(), std::move(quad_floor));

}

} // namespace lve
