#include "lve_model.hpp"
#include <cassert>
#include <cstring>

namespace lve{


       LveModel::LveModel(LveDevice &device, const std::vector<Vertex> &vertices) : lveDevice{device}{
            createVertexBuffers(vertices);
        }
        LveModel::~LveModel(){
            vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
            vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);
        }
        void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices){
            vertexCount = static_cast<uint32_t>(vertices.size());
            assert(vertexCount >=3 && "Vertex count must be at least 3");
            VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
            lveDevice.createBuffer(
                bufferSize,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                //host is cpu, device is gpu
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                vertexBuffer,
                vertexBufferMemory);

            void *data;
            //first 0 is offset,
            //second 0 is for not providing map flag(?)
            vkMapMemory(lveDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
            vkUnmapMemory(lveDevice.device(), vertexBufferMemory);
        }

        void LveModel::draw(VkCommandBuffer commandBuffer){
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
        void LveModel::bind(VkCommandBuffer commandBuffer){
            VkBuffer buffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        }

        std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescription(){
            std::vector<VkVertexInputBindingDescription> bindingDescription(1);
            bindingDescription[0].binding = 0;
            bindingDescription[0].stride = sizeof(Vertex);
            bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescription;
        }
        std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescription(){
            std::vector<VkVertexInputAttributeDescription> attributeDescription(1);
            attributeDescription[0].binding = 0;
            attributeDescription[0].location = 0;
            attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescription[0].offset = 0;
            return attributeDescription;
        }
}