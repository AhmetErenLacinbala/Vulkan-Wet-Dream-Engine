#pragma once

#include "lve_camera.hpp"
#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_game_object.hpp"
#include "lve_frame_info.hpp"

#include <memory>
#include <vector>


namespace lve {

    class PointLightSytem {
        public:
        PointLightSytem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSytem();
        PointLightSytem(const PointLightSytem&) = delete;
        PointLightSytem& operator=(const PointLightSytem&) = delete;
        void render(FrameInfo& frameInfo);
    
    private:

        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        LveDevice &lveDevice;
        
        std::unique_ptr<LvePipeline>lvePipeline;
        VkPipelineLayout pipelineLayout;
    
    };
}