//
// Created by volts on 2026-08-23.
//

#include "Renderer.h"


#include <cstdio>
#include <cstring>
#include <stb_image.h>
#include <tracy/Tracy.hpp>

#include "Buffer.h"
#include "Pipeline.h"


namespace Caelus {
    void Renderer::resizeSwapChain(GLFWwindow* window) {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        // If the window is minimizes, wait until it isn't.
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_context.device);

        vkDestroyImageView(m_context.device, m_swapChain.depthView, nullptr);
        vkDestroyImage(m_context.device, m_swapChain.depthImage, nullptr);
        vkFreeMemory(m_context.device, m_swapChain.depthMemory, nullptr);

        for (const auto v : m_swapChain.views) vkDestroyImageView(m_context.device, v, nullptr);
        vkDestroySwapchainKHR(m_context.device, m_swapChain.handle, nullptr);

        if (!createSwapChain(m_context.gpu, m_context.surface, m_context.device, m_swapChain)) {
            printf("Failed to resize swapChain!\n");
            return;
        }
        printf("Successfully resized swapChain!\n");
    }

    Buffer Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const void* data) {
        Buffer staging{};
        if (!allocateBuffer(m_context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging)) {
            printf("Failed to allocate buffer!\n");
            return {};
        }

        void* mapped = nullptr;
        vkMapMemory(m_context.device, staging.memory, 0, size, 0, &mapped);
        memcpy(mapped, data, static_cast<size_t>(size));
        vkUnmapMemory(m_context.device, staging.memory);

        Buffer b;
        if (!allocateBuffer(m_context, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, b)) {
            destroyBuffer(staging);
            printf("Failed to allocate buffer!\n");
            return {};
        }

        VkCommandBuffer cmd = beginOneTimeCommand();

        VkBufferCopy region{.srcOffset = 0, .dstOffset = 0, .size = size};
        vkCmdCopyBuffer(cmd, staging.handle, b.handle, 1, &region);

        VkBufferMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = b.handle;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;

        VkDependencyInfo dep{};
        dep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep.bufferMemoryBarrierCount = 1;
        dep.pBufferMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(cmd, &dep);

        endOneTimeCommand(cmd);

        destroyBuffer(staging);
        return b;
    }

    void Renderer::destroyBuffer(Buffer& buffer) {
        Caelus::destroyBuffer(m_context.device, buffer); // the Caelus:: is needed, see earlier
    }

    VkCommandBuffer Renderer::beginOneTimeCommand() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_context.device, &allocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);
        return cmd;
    }

    void Renderer::endOneTimeCommand(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkCommandBufferSubmitInfo cbInfo{};
        cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cbInfo.commandBuffer = commandBuffer;

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cbInfo;

        vkQueueSubmit2(m_context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_context.graphicsQueue);

        vkFreeCommandBuffers(m_context.device, m_commandPool, 1, &commandBuffer);
    }

    Textures::Texture Renderer::createTexture(const char* path) {
        int width, height, channels;
        stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
        if (!pixels) {
            printf("stb failed: %s\n", stbi_failure_reason());
            return {};
        }

        const Textures::Texture tex = createTexture(pixels, width, height);
        
        stbi_image_free(pixels);
        
        return tex;
    }
    
    Textures::Texture Renderer::createTexture(const unsigned char* pixels, int width, int height) {
        Textures::Texture tex;
        VkDeviceSize size = static_cast<VkDeviceSize>(width) * height * 4;
        tex.width = static_cast<uint32_t>(width);
        tex.height = static_cast<uint32_t>(height);

        Buffer staging{};
        if (!allocateBuffer(m_context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging)) {
            printf("Failed to allocate buffer!\n");
            return {};
        }

        void* mapped = nullptr;
        vkMapMemory(m_context.device, staging.memory, 0, size, 0, &mapped);
        memcpy(mapped, pixels, static_cast<size_t>(size));
        vkUnmapMemory(m_context.device, staging.memory);
        

        VkImageCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ci.imageType = VK_IMAGE_TYPE_2D;
        ci.format = VK_FORMAT_R8G8B8A8_SRGB;
        ci.extent = {.width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height), .depth = 1};
        ci.mipLevels = 1;
        ci.arrayLayers = 1;
        ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(m_context.device, &ci, nullptr, &tex.image) != VK_SUCCESS) {
            printf("Failed to create image!");
            return {};
        }

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(m_context.device, tex.image, &req);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(m_context.gpu, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(m_context.device, &ai, nullptr, &tex.memory) != VK_SUCCESS) {
            printf("Failed to allocate image memory!");
            return {};
        }

        vkBindImageMemory(m_context.device, tex.image, tex.memory, 0);
        
        VkCommandBuffer cmd = beginOneTimeCommand();
        
        VkImageSubresourceRange subRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1
        };
        
        VkImageMemoryBarrier2 toTransfer{};
        toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        toTransfer.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        toTransfer.srcAccessMask = VK_ACCESS_2_NONE;
        toTransfer.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        toTransfer.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toTransfer.image = tex.image;
        toTransfer.subresourceRange = subRange;
        
        VkDependencyInfo dep{};
        dep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep.imageMemoryBarrierCount = 1;
        dep.pImageMemoryBarriers = &toTransfer;
        vkCmdPipelineBarrier2(cmd, &dep);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1};
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {tex.width, tex.height, 1};
        vkCmdCopyBufferToImage(cmd, staging.handle, tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        
        VkImageMemoryBarrier2 toShader = toTransfer;
        toShader.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        toShader.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        toShader.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        toShader.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        toShader.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        toShader.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        dep.pImageMemoryBarriers = &toShader;
        vkCmdPipelineBarrier2(cmd, &dep);
        
        endOneTimeCommand(cmd);
        destroyBuffer(staging);
        
        VkImageViewCreateInfo ici{};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ici.image = tex.image;
        ici.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ici.format = VK_FORMAT_R8G8B8A8_SRGB;
        ici.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0,
            .layerCount = 1
        };

        if (vkCreateImageView(m_context.device, &ici, nullptr, &tex.view) != VK_SUCCESS) {
            printf("Failed to create image view!");
            return {};
        }
        
        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.maxLod = 0;
        
        vkCreateSampler(m_context.device, &sampler, nullptr, &tex.sampler);
        
        VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_textureSetLayout,
        };
        
        VkDescriptorImageInfo imageInfo{
            .sampler = tex.sampler,
            .imageView = tex.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        
        vkAllocateDescriptorSets(m_context.device, &allocInfo, &tex.descriptorSet);
        
        VkWriteDescriptorSet writeDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = tex.descriptorSet,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        
        
        vkUpdateDescriptorSets(m_context.device, 1, &writeDescriptorSet, 0, nullptr);
        
        return tex;
    }
    
    void Renderer::destroyTexture(Textures::Texture& tex) {
        vkDestroySampler(m_context.device, tex.sampler, nullptr);
        vkDestroyImageView(m_context.device, tex.view, nullptr);
        vkDestroyImage(m_context.device, tex.image, nullptr);
        vkFreeMemory(m_context.device, tex.memory, nullptr);
        tex = {};
    }

    Mesh Renderer::createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        Mesh m;
        m.vertexBuffer = createBuffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                      vertices.data());
        m.indexBuffer = createBuffer(indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                     indices.data());
        m.indexCount = static_cast<uint32_t>(indices.size());
        return m;
    }

    void Renderer::destroyMesh(Mesh& m) {
        destroyBuffer(m.vertexBuffer);
        destroyBuffer(m.indexBuffer);
        m = {};
    }

    void Renderer::drawMesh(VkCommandBuffer commandBuffer, Mesh& mesh) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.handle, &offset);
        vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
    }

    Renderer::Renderer(GLFWwindow* window, std::vector<char> vertCode, std::vector<char> fragCode) : m_window(window) {
        VulkanContext context;

        createInstance(context.instance);
        glfwCreateWindowSurface(context.instance, window, nullptr, &context.surface);
        pickGpu(context.instance, context.gpu);
        findGraphicsQueueFamilyIndex(context.gpu, context.queueFamilyIndex, context.surface);
        createLogicalDevice(context.queueFamilyIndex, context.gpu, context.device, context.graphicsQueue);

        SwapChain swapChain;
        createSwapChain(context.gpu, context.surface, context.device, swapChain);

        m_context = context;
        m_swapChain = swapChain;

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_context.queueFamilyIndex;

        if (vkCreateCommandPool(m_context.device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            printf("Failed to create command pool!\n");
        }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_context.device, &allocInfo, &m_cmd) != VK_SUCCESS) {
            printf("Failed to allocate command buffers!\n");
        }

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(m_context.device, &fenceInfo, nullptr, &m_inFlight) != VK_SUCCESS) {
            printf("Failed to create fence!\n");
        }

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(m_context.device, &semaphoreInfo, nullptr, &m_imageAvailable) != VK_SUCCESS) {
            printf("Failed to create semaphore!\n");
        }

        m_renderFinished = std::vector<VkSemaphore>(m_swapChain.images.size());
        for (auto& s : m_renderFinished) { vkCreateSemaphore(m_context.device, &semaphoreInfo, nullptr, &s); }

        VkShaderModule vertModule = createShaderModule(m_context.device, vertCode);
        VkShaderModule fragModule = createShaderModule(m_context.device, fragCode);
        
        VkDescriptorSetLayoutBinding binding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        };
        
        VkDescriptorSetLayoutCreateInfo layoutInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &binding,
        };
        
        VkDescriptorPoolSize descriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 16
        };
        
        VkDescriptorPoolCreateInfo descriptorPoolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, 
            .maxSets = 16, 
            .poolSizeCount = 1,
            .pPoolSizes = &descriptorPoolSize,
        };
        
        vkCreateDescriptorPool(m_context.device, &descriptorPoolInfo, nullptr, &m_descriptorPool);
        vkCreateDescriptorSetLayout(m_context.device, &layoutInfo, nullptr, &m_textureSetLayout);

        createPipeline(vertModule, fragModule, m_context.device, m_swapChain.format, m_pipelineLayout, m_pipeline,
                       m_range, m_textureSetLayout);

        vkDestroyShaderModule(m_context.device, vertModule, nullptr);
        vkDestroyShaderModule(m_context.device, fragModule, nullptr);
    }

    VkCommandBuffer Renderer::beginFrame(const float clearColor[4]) {
        {
            ZoneScopedN("Wait (vsync/GPU)");
            vkWaitForFences(m_context.device, 1, &m_inFlight, VK_TRUE, UINT64_MAX);
            if (vkAcquireNextImageKHR(m_context.device, m_swapChain.handle, UINT64_MAX, m_imageAvailable,
                                      VK_NULL_HANDLE,
                                      &m_imageIndex) == VK_ERROR_OUT_OF_DATE_KHR) {
                resizeSwapChain(m_window);
                return VK_NULL_HANDLE;
            }
        }

        vkResetFences(m_context.device, 1, &m_inFlight);
        vkResetCommandBuffer(m_cmd, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(m_cmd, &beginInfo);

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier.srcAccessMask = 0;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_swapChain.images[m_imageIndex];
        barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0,
            .layerCount = 1
        };

        VkImageMemoryBarrier2 depthBarrier{};
        depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        depthBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        depthBarrier.srcAccessMask = 0;
        depthBarrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        depthBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.image = m_swapChain.depthImage;
        depthBarrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0,
            .layerCount = 1
        };

        const VkImageMemoryBarrier2 barriers[2] = {barrier, depthBarrier};

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.imageMemoryBarrierCount = 2;
        dependencyInfo.pImageMemoryBarriers = barriers;

        vkCmdPipelineBarrier2(m_cmd, &dependencyInfo);

        VkRenderingAttachmentInfo colorAttachmentInfo{};
        colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo.imageView = m_swapChain.views[m_imageIndex];
        colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentInfo.clearValue.color = {clearColor[0], clearColor[1], clearColor[2], clearColor[3]};

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.imageView = m_swapChain.depthView;
        depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentInfo.clearValue.depthStencil = {.depth = 0.0f, .stencil = 0};
        //? Clear to 0 since reverse z uses 0 as furthest away.

        VkRenderingInfo renderInfo{};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.renderArea = {.offset = {.x = 0, .y = 0}, .extent = m_swapChain.extent};
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachmentInfo;
        renderInfo.pDepthAttachment = &depthAttachmentInfo;

        vkCmdBeginRendering(m_cmd, &renderInfo);

        VkViewport vp{};
        vp.width = static_cast<float>(m_swapChain.extent.width);
        vp.height = static_cast<float>(m_swapChain.extent.height);
        vp.minDepth = 0.0f;
        vp.maxDepth = 1.0f;
        vkCmdSetViewport(m_cmd, 0, 1, &vp);

        VkRect2D scissor{.offset = {.x = 0, .y = 0}, .extent = m_swapChain.extent};
        vkCmdSetScissor(m_cmd, 0, 1, &scissor);


        return m_cmd;
    }

    void Renderer::endFrame() {
        vkCmdEndRendering(m_cmd);

        VkImageMemoryBarrier2 toPresent{};
        toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        toPresent.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        toPresent.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        toPresent.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        toPresent.dstAccessMask = 0;
        toPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toPresent.image = m_swapChain.images[m_imageIndex];
        toPresent.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0,
            .layerCount = 1
        };

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &toPresent;

        dependencyInfo.pImageMemoryBarriers = &toPresent;
        vkCmdPipelineBarrier2(m_cmd, &dependencyInfo);

        vkEndCommandBuffer(m_cmd);

        VkCommandBufferSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        submitInfo.commandBuffer = m_cmd;

        VkSemaphoreSubmitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        waitInfo.semaphore = m_imageAvailable;
        waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSemaphoreSubmitInfo signalInfo{};
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalInfo.semaphore = m_renderFinished[m_imageIndex];
        signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkSubmitInfo2 submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit.waitSemaphoreInfoCount = 1;
        submit.pWaitSemaphoreInfos = &waitInfo;
        submit.commandBufferInfoCount = 1;
        submit.pCommandBufferInfos = &submitInfo;
        submit.signalSemaphoreInfoCount = 1;
        submit.pSignalSemaphoreInfos = &signalInfo;

        vkQueueSubmit2(m_context.graphicsQueue, 1, &submit, m_inFlight);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_renderFinished[m_imageIndex];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain.handle;
        presentInfo.pImageIndices = &m_imageIndex;

        if (VkResult presentResult = vkQueuePresentKHR(m_context.graphicsQueue, &presentInfo); presentResult ==
            VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) { resizeSwapChain(m_window); }
    }

    Renderer::~Renderer() {
        vkDeviceWaitIdle(m_context.device);
        for (const VkSemaphore s : m_renderFinished) vkDestroySemaphore(m_context.device, s, nullptr);
        vkDestroySemaphore(m_context.device, m_imageAvailable, nullptr);
        vkDestroyFence(m_context.device, m_inFlight, nullptr);

        vkDestroyCommandPool(m_context.device, m_commandPool, nullptr); // frees `cmd` too
        vkDestroyImageView(m_context.device, m_swapChain.depthView, nullptr);
        vkDestroyImage(m_context.device, m_swapChain.depthImage, nullptr);
        vkFreeMemory(m_context.device, m_swapChain.depthMemory, nullptr);
        for (const VkImageView v : m_swapChain.views) vkDestroyImageView(m_context.device, v, nullptr);
        vkDestroySwapchainKHR(m_context.device, m_swapChain.handle, nullptr);

        vkDestroyDescriptorPool(m_context.device, m_descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_context.device, m_textureSetLayout, nullptr);
        
        vkDestroyPipeline(m_context.device, m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_context.device, m_pipelineLayout, nullptr);

        vkDestroyDevice(m_context.device, nullptr);
        vkDestroySurfaceKHR(m_context.instance, m_context.surface, nullptr);
        vkDestroyInstance(m_context.instance, nullptr);
    }
} // Caelus
