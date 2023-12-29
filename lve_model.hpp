#pragma once

#include "lve_buffer.hpp"
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
        void loadHeightMap(const std::vector<std::vector<float>>& heightMap);

    };

        LveModel(LveDevice &device, const LveModel::Builder& builder);
        ~LveModel();
        LveModel(const LveModel &) = delete;
        LveModel &operator=(const LveModel &) = delete;

        static std::unique_ptr<LveModel> createModelFromFile(LveDevice &device, const std::string &filepath);
        static std::unique_ptr<LveModel> loadHeightMap(LveDevice &device, const std::vector<std::vector<float>>& heightMap);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:

        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);
        LveDevice &lveDevice;
        
        std::unique_ptr<LveBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<LveBuffer> indexBuffer;
        uint32_t indexCount;
    };
}