#include "lve_pipeline.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

namespace lve
{

    LvePipeline::LvePipeline(LveDevice &device, const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo) : lveDevice{device}
    {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    LvePipeline::~LvePipeline(){
        vkDestroyShaderModule(lveDevice.device(), fragShaderModule, nullptr);
        vkDestroyShaderModule(lveDevice.device(), vertShaderModule, nullptr);
        vkDestroyPipeline(lveDevice.device(), graphicsPipeline, nullptr);
    }

    std::vector<char> LvePipeline::readFile(const std::string &filepath)
    {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file: " + filepath);
        }
        // Read from the end of the file to get its size
        // The reason of using static_cast is making the typecasting safer and readable.
        // The reason of using size_t instead of unsigned int is that size_t is platform independent.
        // size of int may vary from platform to platform.
        // but size_t is always unsigned and is guaranteed to be big enough to contain the size of the biggest object the host system can handle which is 64-bit for a 64-bit system.
        // reason of using tellg() is that it returns the position of the current character in the input stream.
        // since we are already at the end of the file, we can use this to get the size of the file.
        // by using std::ios::ate, we are setting the position of the input stream to the end of the file.
        size_t fileSize = static_cast<size_t>(file.tellg());

        std::vector<char> buffer(fileSize);

        // seekg() is used to set the position of the input stream to the beginning of the file.
        file.seekg(0);

        // Vector.data() returns a pointer to the first element of the vector.
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
    void LvePipeline::createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo)
    {

        assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
        assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass provided in configInfo");
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);
        VkPipelineShaderStageCreateInfo shaderStages[2];

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].pName = "main";
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = NULL;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].pName = "main";
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = NULL;
        shaderStages[0].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.viewportInfo; 
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineInfo.pDynamicState = nullptr;

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if(vkCreateGraphicsPipelines(lveDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)!= VK_SUCCESS){
            throw std::runtime_error("failed to create graphics pipeline");
        }

    }

    void LvePipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }
    }
    PipelineConfigInfo LvePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
    {
        PipelineConfigInfo configInfo{};
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        // This tells the api vertecis  are going to be a triangle, for example not a line
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // This tells the api to not assemble the vertices into lines or points

        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // Triangle strip is a way to draw a triangle with less vertices
        // With the triangle stript method you use the common vertices of nearby triangles
        // So for example you can draw 2 triangles with 4 vertices instead of 6

        // Triangle fan is another way to draw a triangle with less vertices
        // triangle list is the most basic way to draw a triangle

        // Viewport describes the transformation between pipeline's output and the image.
        // So the viewport tells the pipeline how we position our gl_position values to the output image.
        // The viewport is the region of the framebuffer that the output will be rendered to.
        // For example if you increase the widht value, image will be squeezed horizontally.
        configInfo.viewport.x = 0.0f;
        configInfo.viewport.y = 0.0f;
        configInfo.viewport.width = static_cast<float>(width);
        configInfo.viewport.height = static_cast<float>(height);
        configInfo.viewport.minDepth = 0.0f;
        configInfo.viewport.maxDepth = 1.0f;

        // Scissor is used to cut out certain parts of the image.
        configInfo.scissor.offset = {0, 0};
        configInfo.scissor.extent = {width, height};

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = &configInfo.viewport;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = &configInfo.scissor;

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // If depthClampEnable is set to VK_TRUE, Z component of gl_position set between 0 and 1. If it is less then 0, it clamps to 0. If it is more than 1, it clamps to 1.
        // If depthClampEnable is set to VK_FALSE, then the fragments that are beyond the near and far planes are discarded.
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;

        // cullMode is used to specify which face should be culled.
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional




        configInfo.multisampleInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable=VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading=1.0f;            // Optional
        configInfo.multisampleInfo.pSampleMask=nullptr;              // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable=VK_FALSE;   // Optional
        configInfo.multisampleInfo.alphaToOneEnable=VK_FALSE;        // Optional

        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;      //Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;     //Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                 //Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;      //Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;     //Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                 //Optional



        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;           // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;             // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0;              // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;             // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;     // Optional

        return configInfo;
    }
}