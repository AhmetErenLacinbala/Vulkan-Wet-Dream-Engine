#pragma once


#include "lve_device.hpp"
#include <string>
#include <vector>

namespace lve
{

    struct PipelineConfigInfo{

    };

    class LvePipeline
    {
        public:
            LvePipeline(LveDevice &device ,const std::string& vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo& configInfo);

            ~LvePipeline(){}
            LvePipeline(const LvePipeline&) = delete; // not copyable
            void operator=(const LvePipeline&) = delete; // not copyable

            static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
            
        private:
        static std::vector<char> readFile(const std::string& filepath);
        void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        LveDevice& lveDevice; // reference to logical device which the pipeline will be used with
        VkPipeline graphicsPipeline; // handle to graphics pipeline object
        VkShaderModule vertShaderModule; // handle to vertex shader module object
        VkShaderModule fragShaderModule; // handle to fragment shader module object
        //actually all 3 above are the pointers to the objects in the GPU memory
    };
} // namespace lve