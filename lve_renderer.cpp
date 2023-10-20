#include "lve_renderer.hpp"

#include <glm/gtc/constants.hpp>
#include <iostream>
#include <stdexcept>

namespace lve {

LveRenderer::LveRenderer(LveWindow &window, LveDevice &device) : lveWindow{window}, lveDevice{device} {
    recreateSwapChain();
    createCommandBuffers();
}

LveRenderer::~LveRenderer() {
    freeCommandBuffers();
}

void LveRenderer::recreateSwapChain() {
    auto extent = lveWindow.getExtend();
    while (extent.width == 0 || extent.height == 0) {
        extent = lveWindow.getExtend();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(lveDevice.device());

    if (lveSwapChain == nullptr) {
        lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
    } else {
        std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
        lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);
        if(!oldSwapChain->compareSwapFormats(*lveSwapChain.get())){
            throw std::runtime_error("Swap chain image (or depth) format has changed");
        }
    }
}

void LveRenderer::createCommandBuffers() {
    // resize command buffer count to have one for each image in swap chain
    //.imageCount() returns the number of images in the swap chain
    // imageCount will likely be 2 or 3 depends on my device supports double or triple buffering
    // apple silicone m1 and m2 supports triple buffering

    // each command buffer is going to draw to a different frame buffer here
    commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

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

void LveRenderer::freeCommandBuffers() {
    vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}


VkCommandBuffer LveRenderer::beginFrame() {
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = lveSwapChain->acquireNextImage(&currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }
    isFrameStarted = true;
    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to being recording command buffer");
    }
    return commandBuffer;
}
void LveRenderer::endFrame() {
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer");
    }

    auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
        lveWindow.resetWindowResizeFlag();
        recreateSwapChain();
    }

    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image");
    }
    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex+1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
}
void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = lveSwapChain->getRenderPass();
    renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);
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

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

}
void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
     assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);

}

} // namespace lve
