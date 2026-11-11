//
// Created by volts on 2026-08-23.
//

#ifndef CAELUSENGINE_RENDERER_H
#define CAELUSENGINE_RENDERER_H

#include <vulkan/vulkan.h>
#include "GLFW/glfw3.h"
#include <vector>

#include "Buffer.h"
#include "SwapChain.h"
#include "VulkanContext.h"
#include "Math/Math.h"

namespace Caelus {
    class Renderer {
    public:
        explicit Renderer(GLFWwindow* window, std::vector<char> vertCode, std::vector<char> fragCode);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        [[nodiscard]] const GLFWwindow* window() const { return m_window; }
        [[nodiscard]] const VulkanContext& context() const { return m_context; }
        [[nodiscard]] const SwapChain& swapChain() const { return m_swapChain; }
        [[nodiscard]] const VkCommandPool& commandPool() const { return m_commandPool; }
        [[nodiscard]] const VkCommandBuffer& commandBuffer() const { return m_cmd; }
        [[nodiscard]] const VkFence& inFlight() const { return m_inFlight; }
        [[nodiscard]] const VkSemaphore& imageAvailable() const { return m_imageAvailable; }
        [[nodiscard]] const std::vector<VkSemaphore>& renderFinished() const { return m_renderFinished; }
        [[nodiscard]] uint32_t imageIndex() const { return m_imageIndex; }
        [[nodiscard]] VkPipeline pipeline() const { return m_pipeline; }
        [[nodiscard]] VkPipelineLayout pipelineLayout() const { return m_pipelineLayout; }

        VkCommandBuffer beginFrame(const float clearColor[4]);
        void endFrame();

        void resizeSwapChain(GLFWwindow* window);
        
        Buffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const void* data);
        void destroyBuffer(Buffer& buffer);

    private:
        GLFWwindow* m_window = nullptr;
        VulkanContext m_context;
        SwapChain m_swapChain;

        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkCommandBuffer m_cmd = VK_NULL_HANDLE;
        VkFence m_inFlight = VK_NULL_HANDLE;
        VkSemaphore m_imageAvailable = VK_NULL_HANDLE;
        std::vector<VkSemaphore> m_renderFinished;

        VkPushConstantRange m_range;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;

        uint32_t m_imageIndex = 0;
        
        VkCommandBuffer beginOneTimeCommand();
        void endOneTimeCommand(VkCommandBuffer commandBuffer);
    };
} // Caelus

#endif //CAELUSENGINE_RENDERER_H
