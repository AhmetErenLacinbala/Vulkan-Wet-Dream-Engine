#pragma once

#include "lve_device.hpp"

#include <glm/glm.hpp>

namespace lve
{
    class LveModel
    {
    public:
        LveModel();
        ~LveModel();
        LveModel(const LveModel &) = delete;
        LveModel &operator=(const LveModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:
        LveDevice &LveDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}