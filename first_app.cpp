#include "first_app.hpp"

#include <stdexcept>

namespace lve {

    FirstApp::FirstApp(){
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

       FirstApp::~FirstApp(){
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void FirstApp::run(){
        while(!lveWindow.shouldClose()){
            //poll events checks if any events are triggered (like keyboard or mouse input)
            //or dismissed the window etc.
            glfwPollEvents();
            drawFrame();

            vkDeviceWaitIdle(lveDevice.device());
        }
    }
    void FirstApp::createPipelineLayout(){
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout");
        } 
    }


        void FirstApp::createPipeline(){
            auto pipelineConfig = LvePipeline::defaultPipelineConfigInfo(lveSwapChain.width(),lveSwapChain.height());
            pipelineConfig.renderPass = lveSwapChain.getRenderPass();
            pipelineConfig.pipelineLayout = pipelineLayout;

            lvePipeline = std::make_unique<LvePipeline>(
                lveDevice,
                "shaders/simple_shader.vert.spv",
                "shaders/simple_shader.frag.spv",
                pipelineConfig
            );
        }


        void FirstApp::createCommandBuffers(){
            //resize command buffer count to have one for each image in swap chain
            //.imageCount() returns the number of images in the swap chain
            //imageCount will likely be 2 or 3 depends on my device supports double or triple buffering
            //apple silicone m1 and m2 supports triple buffering

            //each command buffer is going to draw to a different frame buffer here
            commandBuffers.resize(lveSwapChain.imageCount());

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            //there are 2 types command buffers: primary and secondary
            //primary can be submitted to a queue for execution, but cannot be called from other command buffers
            //secondary cannot be submitted directly, but can be called from other command buffers
            //with vkCmdExecuteCommands command from primary command buffer secondary command buffers can be called by other command buffers
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = lveDevice.getCommandPool();
            allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

            if(vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data())!=VK_SUCCESS){
                throw std::runtime_error("failed to allocate command buffers");
            }

            for (int i=0; i<commandBuffers.size(); i++){
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                
                if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS){
                    throw std::runtime_error("failed to being recording command buffer");
                }
                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = lveSwapChain.getRenderPass();
                renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);
                renderPassInfo.renderArea.offset = {0,0};
                renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

                std::array<VkClearValue, 2>clearValues{};
                //in the render pass attachments are structured with index
                //index 0 is the color attachment
                //index 1 is the depth attachment
                //so we are not usins clearValues[0].depthStencil
                clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
                clearValues[1].depthStencil = {1.0f, 0};
                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

            
                lvePipeline->bind(commandBuffers[i]);

                //this records a draw command to draw three vertices and only one instances
                //instance can be used when you want to draw multiple copies of the same object.
                //this is very handy when working with particals.
                //0, 0 because we don't use any offest currently.
                vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

                vkCmdEndRenderPass(commandBuffers[i]);
                if(vkEndCommandBuffer(commandBuffers[i]) !=VK_SUCCESS){
                    throw std::runtime_error("failed to record command buffer");
                }
            }

        }


        void FirstApp::drawFrame(){
            uint32_t imageIndex;
            auto result= lveSwapChain.acquireNextImage(&imageIndex);
            
            if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
                throw std::runtime_error("failed to acquire swap chain image");
            }
            result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
            if(result != VK_SUCCESS){
                throw std::runtime_error("failed to present swap chain image");
            }
        }






}

