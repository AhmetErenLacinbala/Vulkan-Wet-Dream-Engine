#include "first_app.hpp"

#include <iostream>
#include <stdexcept>
#include <glm/gtc/constants.hpp>

namespace lve {

FirstApp::FirstApp() {
    loadGameObjects();
    createPipelineLayout();
    recreateSwapChain();
    createCommandBuffers();
}

FirstApp::~FirstApp() {
    vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void FirstApp::run() {

    std::cout << "max push conts size = " << lveDevice.properties.limits.maxPushConstantsSize << "\n";
    while (!lveWindow.shouldClose()) {
        // poll events checks if any events are triggered (like keyboard or mouse input)
        // or dismissed the window etc.
        glfwPollEvents();
        drawFrame();

        vkDeviceWaitIdle(lveDevice.device());
    }
}

void FirstApp::loadGameObjects() {
    std::vector<LveModel::Vertex> vertices{
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}

    };

    auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

    auto triangle = LveGameObject::createGameObject();
    triangle.model = lveModel;
    triangle.color = {.1f, .8f, .1f};
    triangle.transform2d.translation.x = 0.2f;
    triangle.transform2d.scale = {2.f, .5f};
    triangle.transform2d.rotation = .25f * glm::two_pi<float>();

    gameObjects.push_back(std::move(triangle));
    

}
void FirstApp::createPipelineLayout() {

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

void FirstApp::recreateSwapChain() {
    auto extent = lveWindow.getExtend();
    while (extent.width == 0 || extent.height == 0) {
        extent = lveWindow.getExtend();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(lveDevice.device());

    if (lveSwapChain == nullptr) {
        lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
    } else {
        lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
        if (lveSwapChain->imageCount() != commandBuffers.size()) {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }

    createPipeline();
}

void FirstApp::createPipeline() {
    assert(lveSwapChain != nullptr && "cannot create pipeline before swap chain");
    assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");
    PipelineConfigInfo pipelineConfig{};
    LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = lveSwapChain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;

    lvePipeline = std::make_unique<LvePipeline>(
        lveDevice,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        pipelineConfig);
}

void FirstApp::createCommandBuffers() {
    // resize command buffer count to have one for each image in swap chain
    //.imageCount() returns the number of images in the swap chain
    // imageCount will likely be 2 or 3 depends on my device supports double or triple buffering
    // apple silicone m1 and m2 supports triple buffering

    // each command buffer is going to draw to a different frame buffer here
    commandBuffers.resize(lveSwapChain->imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // there are 2 types command buffers: primary and secondary
    // primary can be submitted to a queue for execution, but cannot be called from other command buffers
    // secondary cannot be submitted directly, but can be called from other command buffers
    // with vkCmdExecuteCommands command from primary command buffer secondary command buffers can be called by other command buffers
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = lveDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers");
    }
}

void FirstApp::freeCommandBuffers() {
    vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

void FirstApp::recordCommandBuffer(int imageIndex) {

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to being recording command buffer");
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = lveSwapChain->getRenderPass();
    renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    // in the render pass attachments are structured with index
    // index 0 is the color attachment
    // index 1 is the depth attachment
    // so we are not usins clearValues[0].depthStencil
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    /*
    --inline:
    VK_SUBPASS_CONTENTS_INLINE signals that subseqeunt render pass commands will be directly embedded in primary command buffer itself.
    and that no secondary command buffers will be used.
    -primary command buffer:
    *Start render pass
        Bind pipeline 1
        Bind texture 4
        Draw 100 Vertices
    *End render pass

    --Secondary command buffers (alternative):
    signaling that render pass commands will be executed secondary command buffers with VkCmdExecute command
    *Start render pass
        VkCmdExecute -> secondary cmd buffer
        VkCmdExecute -> secondary cmd buffer
    *End render pass

    You can't mix these to ways.
    */

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

    renderGameObjects(commandBuffers[imageIndex]);

    vkCmdEndRenderPass(commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer");
    }
}


void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer){
    lvePipeline->bind(commandBuffer);
    for(auto &obj: gameObjects){
        obj.transform2d.rotation = glm::mod(obj.transform2d.rotation +0.01f , glm::two_pi<float>());
        SimplePushConstantData push{};
        push.offset = obj.transform2d.translation;
        push.color = obj.color;
        push.transform = obj.transform2d.mat2();

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}

void FirstApp::drawFrame() {
    uint32_t imageIndex;
    auto result = lveSwapChain->acquireNextImage(&imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }
    recordCommandBuffer(imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
        lveWindow.resetWindowResizeFlag();
        recreateSwapChain();
        return;
    }

    result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image");
    }
}

} // namespace lve
