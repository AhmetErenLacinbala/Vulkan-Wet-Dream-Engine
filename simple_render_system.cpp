#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <stdexcept>

struct SimplePushConstantData{
    glm::mat4 transform{1.f};
    alignas(16) glm::vec3 color;
};


namespace lve {

SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass) : lveDevice{device} {
    
    createPipelineLayout();
    createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
    vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}



void SimpleRenderSystem::createPipelineLayout() {

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    // offset is for if you use seperate vertex and fragment bit. It is 0 since we use them together
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }
}


void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
    assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");
    PipelineConfigInfo pipelineConfig{};
    LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;

    lvePipeline = std::make_unique<LvePipeline>(
        lveDevice,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        pipelineConfig);
}


void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects, const LveCamera& camera){
    lvePipeline->bind(commandBuffer);
    for(auto &obj: gameObjects){
        obj.transform.rotation.y = glm::mod(obj.transform.rotation.y +0.01f , glm::two_pi<float>());
        obj.transform.rotation.x = glm::mod(obj.transform.rotation.y +0.005f , glm::two_pi<float>());
        SimplePushConstantData push{};
        push.color = obj.color;
        // most of the game engines don't handle the projection on cpu
        // they handle it on gpu through shaders instead
        push.transform = camera.getProjection() * obj.transform.mat4();

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}


} // namespace lve
