//
// Created by volts on 2026-08-23.
//

#include "Renderer.h"

#include <cstdio>



namespace Caelus {
    static VkShaderModule createShaderModule(const VkDevice &device, const std::vector<char> &code) {
        VkShaderModuleCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ci.codeSize = code.size();
        ci.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &ci, nullptr, &shaderModule) != VK_SUCCESS) {
            printf("Failed to create shader module!\n");
            return VK_NULL_HANDLE;
        }
        return shaderModule;
    }

    static uint32_t findMemoryType(const VkPhysicalDevice &gpu, const uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if ((typeFilter & (1u << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        printf("Failed to find suitable memory type!\n");
        return 0;
    }

    static bool createDepthBuffer(const VkPhysicalDevice &gpu, const VkDevice &device, SwapChain &swapChain) {
        VkImageCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ci.imageType = VK_IMAGE_TYPE_2D;
        ci.format = swapChain.depthFormat;
        ci.extent = {.width = swapChain.extent.width, .height = swapChain.extent.height, .depth = 1};
        ci.mipLevels = 1;
        ci.arrayLayers = 1;
        ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(device, &ci, nullptr, &swapChain.depthImage) != VK_SUCCESS) {
            printf("Failed to create depth image!");
            return false;
        }

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(device, swapChain.depthImage, &req);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(gpu, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &ai, nullptr, &swapChain.depthMemory) != VK_SUCCESS) {
            printf("Failed to allocate depth memory!");
            return false;
        }

        vkBindImageMemory(device, swapChain.depthImage, swapChain.depthMemory, 0);

        VkImageViewCreateInfo ici{};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ici.image = swapChain.depthImage;
        ici.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ici.format = swapChain.depthFormat;
        ici.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

        if (vkCreateImageView(device, &ici, nullptr, &swapChain.depthView) != VK_SUCCESS) {
            printf("Failed to create depth view!");
            return false;
        }
        printf("Successfully created depth buffer!");
        return true;
    }

    static bool createSwapChain(const VkPhysicalDevice &gpu, const VkSurfaceKHR &surface, const VkDevice &device, SwapChain &out) {
        //! GET INFORMATION ABOUT THE SWAPCHAIN
        // Get some information Idk
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &caps);

        // Which pixel format it supports, call twice like in pickGpu()
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, &formats[0]);

        // Get the correct format
        VkSurfaceFormatKHR chosenFormat = formats[0];
        for (const auto &format: formats) {
            if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                chosenFormat = format;
                break;
            }
        }

        // How many images to keep in buffer so the cpu doesnt have to wait
        uint32_t imageCount = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
            imageCount = caps.maxImageCount;

        const VkExtent2D extent = caps.currentExtent;

        //! CREATE THE SWAPCHAIN

        VkSwapchainCreateInfoKHR ci{};
        ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        ci.surface = surface;
        ci.minImageCount = imageCount;
        ci.imageFormat = chosenFormat.format;
        ci.imageColorSpace = chosenFormat.colorSpace;
        ci.imageExtent = extent;
        ci.imageArrayLayers = 1;
        ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ci.preTransform = caps.currentTransform;
        ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Dont blend the window with the desktop
        ci.presentMode = VK_PRESENT_MODE_FIFO_KHR; // VSync
        ci.clipped = VK_TRUE;
        ci.oldSwapchain = VK_NULL_HANDLE;


        if (vkCreateSwapchainKHR(device, &ci, nullptr, &out.handle) != VK_SUCCESS) {
            printf("Failed to create swapChain!\n");
            return false;
        }
        out.format = chosenFormat.format;
        out.extent = extent;
        printf("Successfully created swapchain! width: %u height: %u \n", extent.width, extent.height);

        //! CREATE IMAGES AND VIEWS

        // Get the amount of images, same double call as always
        uint32_t imgCount = 0;
        vkGetSwapchainImagesKHR(device, out.handle, &imgCount, nullptr);

        out.images.resize(imgCount);
        out.views.resize(imgCount);
        vkGetSwapchainImagesKHR(device, out.handle, &imgCount, out.images.data());

        // Create one image view per image
        for (uint32_t i = 0; i < imgCount; i++) {
            VkImageViewCreateInfo ici{};
            ici.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ici.image = out.images[i];
            ici.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ici.format = out.format;
            ici.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            // Color data as opposed to something like a depth buffer
            ici.subresourceRange.baseMipLevel = 0;
            ici.subresourceRange.levelCount = 1;
            ici.subresourceRange.baseArrayLayer = 0;
            ici.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &ici, nullptr, &out.views[i]) != VK_SUCCESS) {
                printf("Failed to create image view %u!\n", i);
                return false;
            }
        }
        printf("Successfully created %u image views!\n", imgCount);

        if (!createDepthBuffer(gpu, device, out)) return false;

        return true;
    }

    static bool createInstance(VkInstance &instance) {
        // Tells the driver which vulkan version we are using
        VkApplicationInfo app{};
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.apiVersion = VK_API_VERSION_1_3;

        // Needed to allow vulkan to interact with the window
        uint32_t extCount = 0;
        const char **exts = glfwGetRequiredInstanceExtensions(&extCount);

        // The "recipe" for our vulkan Instance
        VkInstanceCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ci.pApplicationInfo = &app;
        ci.enabledExtensionCount = extCount;
        ci.ppEnabledExtensionNames = exts;

        // Enables debug for vulkan. ifndef makes sure to only enable if debugging since it impacts performance
        #ifndef NDEBUG
        const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
        ci.enabledLayerCount = 1;
        ci.ppEnabledLayerNames = layers;
        #endif


        // Creates the vulkan Instance
        if (vkCreateInstance(&ci, nullptr, &instance) != VK_SUCCESS) {
            printf("Failed to create Vulkan instance!\n");
            return false;
        }
        printf("Vulkan instance created!\n");
        return true;
    }

    static bool pickGpu(const VkInstance &instance, VkPhysicalDevice &gpu) {
        // Finds all gpus
        // call two times, once to get the amount and once to fill an array.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Sorts through devices to find one that is an actual gpu
        for (const auto &device: devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);
            printf("Device name: %s\n", props.deviceName);

            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                gpu = device;
                return true;
            }
        }

        printf("Failed to find suitable GPU!\n");
        return false;
    }

    //? returns the index of the queue because the queue doesn't exist yet
    static bool findGraphicsQueueFamilyIndex(const VkPhysicalDevice &gpu, uint32_t &queueFamilyIndex, const VkSurfaceKHR &surface) {
        // find gpu queue families
        // Run twice, same as in pickGpu()
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, nullptr);

        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, families.data());

        // Find which family can draw and present
        for (uint32_t i = 0; i < familyCount; i++) {
            const bool canDraw = families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            VkBool32 canPresent = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &canPresent);
            if (canDraw && canPresent) {
                printf("Graphics queue family index: %u\n", i);
                queueFamilyIndex = i;
                return true;
            }
        }
        printf("Could not find graphics queue family index!\n");
        return false;
    }

    static bool createLogicalDevice(const uint32_t& queueFamilyIndex, const VkPhysicalDevice &gpu, VkDevice &device, VkQueue &graphicsQueue) {
        constexpr float priority = 1.0f;

        // The "recipe" for the device queue
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &priority;

        // What features we want enabled
        VkPhysicalDeviceVulkan13Features features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features.dynamicRendering = VK_TRUE;
        features.synchronization2 = VK_TRUE;

        const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        // The "recipe" for the device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &features;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = deviceExtensions;

        // Create the device
        if (vkCreateDevice(gpu, &createInfo, nullptr, &device) != VK_SUCCESS) {
            printf("Failed to create logical device!\n");
            return false;
        }

        // retrieve the graphicsQueue
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &graphicsQueue);
        printf("successfully created logical device!\n");
        return true;
    }

    static bool createPipeline(const VkShaderModule& vertModule, const VkShaderModule& fragModule, const VkDevice& device, const VkFormat format, VkPipelineLayout& layout, VkPipeline& pipeline, VkPushConstantRange& range) {
        VkPipelineShaderStageCreateInfo stages[2]{};

        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vertModule;
        stages[0].pName = "main";

        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fragModule;
        stages[1].pName = "main";

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationInfo.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                         VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
        colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingInfo.attachmentCount = 1;
        colorBlendingInfo.pAttachments = &blendAttachment;

        VkDynamicState dynamics[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicInfo{};
        dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicInfo.dynamicStateCount = 2;
        dynamicInfo.pDynamicStates = dynamics;

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        range.offset = 0;
        range.size = sizeof(Consts);
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &range;

        vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout);

        VkPipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachmentFormats = &format;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &rasterizationInfo;
        pipelineInfo.pMultisampleState = &multisampleInfo;
        pipelineInfo.pColorBlendState = &colorBlendingInfo;
        pipelineInfo.pDynamicState = &dynamicInfo;
        pipelineInfo.layout = layout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            printf("Failed to create graphics pipeline!\n");
            return false;
        }
        printf("Pipeline created!\n");
        return true;
    }

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

        for (const auto v : m_swapChain.views)
            vkDestroyImageView(m_context.device, v, nullptr);
        vkDestroySwapchainKHR(m_context.device, m_swapChain.handle, nullptr);

        if (!createSwapChain(m_context.gpu, m_context.surface, m_context.device, m_swapChain)) {printf("Failed to resize swapChain!\n"); return;}
        printf("Successfully resized swapChain!\n");
    }

    Renderer::Renderer(GLFWwindow *window, std::vector<char> vertCode, std::vector<char> fragCode) : m_window(window) {
        VulkanContext context;

        createInstance(context.instance);
        glfwCreateWindowSurface(context.instance, window, nullptr, &context.surface);
        pickGpu(context.instance, context.gpu);
        findGraphicsQueueFamilyIndex(context.gpu, context.queueFamilyIndex, context.surface);
        createLogicalDevice(context.queueFamilyIndex, context.gpu, context.device, context.graphicsQueue);

        SwapChain swapChain;
        createSwapChain(context.gpu, context.surface, context.device, swapChain );

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
        for (auto &s: m_renderFinished) {
            vkCreateSemaphore(m_context.device, &semaphoreInfo, nullptr, &s);
        }

        VkShaderModule vertModule = createShaderModule(m_context.device, vertCode);
        VkShaderModule fragModule = createShaderModule(m_context.device, fragCode);

        createPipeline(vertModule, fragModule, m_context.device, m_swapChain.format,m_pipelineLayout,m_pipeline, m_range);

        vkDestroyShaderModule(m_context.device, vertModule, nullptr);
        vkDestroyShaderModule(m_context.device, fragModule, nullptr);
    }

    VkCommandBuffer Renderer::beginFrame(const float clearColor[4]) {
        vkWaitForFences(m_context.device, 1, &m_inFlight, VK_TRUE, UINT64_MAX);
        if (vkAcquireNextImageKHR(m_context.device, m_swapChain.handle, UINT64_MAX, m_imageAvailable, VK_NULL_HANDLE, &m_imageIndex) == VK_ERROR_OUT_OF_DATE_KHR) {
            resizeSwapChain(m_window);
            return VK_NULL_HANDLE;
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
        barrier.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(m_cmd, &dependencyInfo);

        VkRenderingAttachmentInfo colorAttachmentInfo{};
        colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo.imageView = m_swapChain.views[m_imageIndex];
        colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentInfo.clearValue.color = {clearColor[0],clearColor[1], clearColor[2],clearColor[3]};

        VkRenderingInfo renderInfo{};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.renderArea = {.offset = {.x = 0,.y = 0}, .extent = m_swapChain.extent};
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachmentInfo;

        vkCmdBeginRendering(m_cmd, &renderInfo);

        VkViewport vp{};
        vp.width = static_cast<float>(m_swapChain.extent.width);
        vp.height = static_cast<float>(m_swapChain.extent.height);
        vp.minDepth = 0.0f;
        vp.maxDepth = 1.0f;
        vkCmdSetViewport(m_cmd, 0, 1, &vp);

        VkRect2D scissor{.offset = {.x = 0,.y = 0}, .extent = m_swapChain.extent};
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
        toPresent.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

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

        if (VkResult presentResult = vkQueuePresentKHR(m_context.graphicsQueue, &presentInfo); presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
            resizeSwapChain(m_window);
        }
    }

    Renderer::~Renderer() {
        vkDeviceWaitIdle(m_context.device);
        for (const VkSemaphore s: m_renderFinished)
            vkDestroySemaphore(m_context.device, s, nullptr);
        vkDestroySemaphore(m_context.device, m_imageAvailable, nullptr);
        vkDestroyFence(m_context.device, m_inFlight, nullptr);

        vkDestroyCommandPool(m_context.device, m_commandPool, nullptr); // frees `cmd` too
        vkDestroyImageView(m_context.device, m_swapChain.depthView, nullptr);
        vkDestroyImage(m_context.device, m_swapChain.depthImage, nullptr);
        vkFreeMemory(m_context.device, m_swapChain.depthMemory, nullptr);
        for (const VkImageView v: m_swapChain.views)
            vkDestroyImageView(m_context.device, v, nullptr);
        vkDestroySwapchainKHR(m_context.device, m_swapChain.handle, nullptr);

        vkDestroyPipeline(m_context.device, m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_context.device, m_pipelineLayout, nullptr);

        vkDestroyDevice(m_context.device, nullptr);
        vkDestroySurfaceKHR(m_context.instance, m_context.surface, nullptr);
        vkDestroyInstance(m_context.instance, nullptr);


    }
} // Caelus