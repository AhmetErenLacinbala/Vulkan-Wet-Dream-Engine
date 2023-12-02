#pragma once

#include "lve_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace lve
{
    class LveModel
    {
    public:

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        static std::vector<VkVertexInputBindingDescription> getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescription();
        glm::vec3 normal{};
        glm::vec2 uv{};

        bool operator ==(const Vertex &other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv; 
        }

    };

    struct Builder {
        std::vector<Vertex> vertices {};
        std::vector<uint32_t> indices {};

        void loadModel(const std::string &filepath);

    };

        LveModel(LveDevice &device, const LveModel::Builder& builder);
        ~LveModel();
        LveModel(const LveModel &) = delete;
        LveModel &operator=(const LveModel &) = delete;

        static std::unique_ptr<LveModel> createModelFromFile(LveDevice &device, const std::string &filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:

        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);
        LveDevice &lveDevice;
        
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        uint32_t indexCount;
    };
}