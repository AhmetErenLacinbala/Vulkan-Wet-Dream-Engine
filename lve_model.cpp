#include "lve_model.hpp"
#include "lve_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std {
template <>
struct hash<lve::LveModel::Vertex> {
    size_t operator()(lve::LveModel::Vertex const &vertex) const {
        size_t seed = 0;
        lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};

} // namespace std

namespace lve {

LveModel::LveModel(LveDevice &device, const LveModel::Builder &builder) : lveDevice{device} {
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}
LveModel::~LveModel() {
}



std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice &device, const std::string &filepath) {
    Builder builder{};
    builder.loadModel(filepath);
    std::cout << "Vertex count: " << builder.vertices.size() << "\n";
    return std::make_unique<LveModel>(device, builder);
}

std::unique_ptr<LveModel> LveModel::loadHeightMap(LveDevice &device, const std::vector<std::vector<float>>& heightMap){

    Builder builder{};
    float scale = 1.0f; // Scale for the grid spacing
    float heightScale = 1.0f; // Scale for the height values

    size_t rows = heightMap.size();
    size_t cols = heightMap.empty() ? 0 : heightMap[0].size();
      for (size_t z = 0; z < rows; ++z) {
        for (size_t x = 0; x < cols; ++x) {
            float height = heightMap[z][x];

            Vertex vertex{};
            vertex.position = glm::vec3(x * scale, height * heightScale, z * scale);
            vertex.color = glm::vec3(1.0f); // Placeholder color
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Placeholder normal
            vertex.uv = glm::vec2(x / static_cast<float>(cols), z / static_cast<float>(rows));

            builder.vertices.push_back(vertex);
        }
    }
        // Generate indices (for a grid of quads)
    for (size_t z = 0; z < rows - 1; ++z) {
        for (size_t x = 0; x < cols - 1; ++x) {
            uint32_t topLeft = z * cols + x;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = (z + 1) * cols + x;
            uint32_t bottomRight = bottomLeft + 1;

            builder.indices.push_back(topLeft);
            builder.indices.push_back(bottomLeft);
            builder.indices.push_back(topRight);

            builder.indices.push_back(topRight);
            builder.indices.push_back(bottomLeft);
            builder.indices.push_back(bottomRight);
        }
    }

    for (size_t z = 0; z < rows; ++z) {
        for (size_t x = 0; x < cols; ++x) {
            glm::vec3 sumNormals(0.0f);

            // Neighbors
            glm::vec3 left = x > 0 ?
                glm::vec3(-1.0f, heightMap[z][x - 1] - heightMap[z][x], 0.0f) : glm::vec3(0.0f);
            glm::vec3 right = x < rows - 1 ?
                glm::vec3(1.0f, heightMap[z][x + 1] - heightMap[z][x], 0.0f) : glm::vec3(0.0f);
            glm::vec3 down = z > 0 ?
                glm::vec3(0.0f, heightMap[z - 1][x] - heightMap[z][x], -1.0f) : glm::vec3(0.0f);
            glm::vec3 up = z < rows - 1 ?
                glm::vec3(0.0f, heightMap[z + 1][x] - heightMap[z][x], 1.0f) : glm::vec3(0.0f);

            // Cross products to compute normals
            if (x > 0 && z > 0) sumNormals += glm::cross(left, down);
            if (x < rows - 1 && z > 0) sumNormals += glm::cross(down, right);
            if (x < rows - 1 && z < rows - 1) sumNormals += glm::cross(right, up);
            if (x > 0 && z < rows - 1) sumNormals += glm::cross(up, left);

            // Assign and normalize the normal
            builder.vertices[z * rows + x].normal = glm::normalize(sumNormals);
        }
    }
    return std::make_unique<LveModel>(device, builder);

}

void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    LveBuffer stagingBuffer{
        lveDevice,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    vertexBuffer = std::make_unique<LveBuffer>(
        lveDevice,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void LveModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
    indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer = indexCount > 0;

    if (!hasIndexBuffer) {
        return;
    }

    assert(vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
    uint32_t indexSize = sizeof(indices[0]);

    LveBuffer stagingBuffer = {
        lveDevice,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)indices.data());

    indexBuffer = std::make_unique<LveBuffer>(
        lveDevice,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void LveModel::draw(VkCommandBuffer commandBuffer) {
    if (hasIndexBuffer) {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
}
void LveModel::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (hasIndexBuffer) {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescription() {
    std::vector<VkVertexInputBindingDescription> bindingDescription(1);
    bindingDescription[0].binding = 0;
    bindingDescription[0].stride = sizeof(Vertex);
    bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}
std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescription() {
    // parameter is about how many attributes we will pass
    // for example 1 is only for position information,
    // but when we use we can pass location and color of the vertices
    std::vector<VkVertexInputAttributeDescription> attributeDescription{};

    attributeDescription.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescription.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescription.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescription.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
    return attributeDescription;
}
void LveModel::Builder::loadModel(const std::string &filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            Vertex vertex{};
            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }
            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            if (index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.vertex_index + 0],
                    attrib.texcoords[2 * index.vertex_index + 1],
                };
            }
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

} // namespace lve