#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <fstream>

struct SwapChain {
    VkSwapchainKHR handle = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent = {};
    std::vector<VkImage> images = {};
    std::vector<VkImageView> views = {};
};

bool initVulkan(VkInstance& instance) {

    // Tells the driver which vulkan version we are using
    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.apiVersion = VK_API_VERSION_1_3;

    // Needed to allow vulkan to interact with the window
    uint32_t extCount = 0;
    const char** exts = glfwGetRequiredInstanceExtensions(&extCount);

    // The "recipe" for our vulkan Instance
    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app;
    ci.enabledExtensionCount = extCount;
    ci.ppEnabledExtensionNames = exts;

    // Enables debug for vulkan. ifndef makes sure to only enable if debugging since it impacts performance
#ifndef NDEBUG
    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    ci.enabledLayerCount   = 1;
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

bool pickGpu(VkInstance& instance, VkPhysicalDevice& gpu) {
    // Finds all gpus
    // call two times, once to get the amount and once to fill an array.
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Sorts through devices to find one that is an actual gpu
    for (auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        printf("Device name: %s\n", props.deviceName);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {gpu = device; return true;}
    }

    printf("Failed to find suitable GPU!\n");
    return false;
}

//? returns the index of the queue because the queue doesn't exist yet
bool findGraphicsQueueFamilyIndex(VkPhysicalDevice& gpu, uint32_t& queueFamilyIndex, VkSurfaceKHR& surface) {
    // find gpu queue families
    // Run twice, same as in pickGpu()
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, families.data());

    // Find which family can draw and present
    for (uint32_t i = 0; i < familyCount; i++) {
        bool canDraw = families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
        VkBool32 canPresent = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &canPresent);
        if (canDraw && canPresent) {printf("Graphics queue family index: %u\n", i); queueFamilyIndex = i; return true;}
    }
    printf("Could not find graphics queue family index!\n");
    return false;
}

bool createLogicalDevice(uint32_t queueFamilyIndex, VkPhysicalDevice& gpu, VkDevice& device, VkQueue& graphicsQueue) {
    float priority = 1.0f;

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

    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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

bool createSwapchain(VkPhysicalDevice& gpu, VkSurfaceKHR& surface, VkDevice& device, SwapChain& out) {

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
    for (const auto& format : formats) {
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

    VkExtent2D extent = caps.currentExtent;

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
        printf("Failed to create swapchain!\n");
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
        ici.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Color data as opposed to something like a depth buffer
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

    return true;
}

static std::vector<char> readFile(const std::string& path) {
    // Get the file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        printf("Failed to open %s\n", path.c_str());
        return {};
    }

    // Get the size of the file
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    // Return the file contents
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    return buffer;
}

static VkShaderModule createShaderModule(VkDevice& device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &ci, nullptr, &shaderModule) != VK_SUCCESS) {
        printf("Failed to create shader module!\n");
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

int main() {

    glfwInit();

    // finds monitors and hooks to the os type shi
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Creates a window >_<
    GLFWwindow* window = glfwCreateWindow(800, 600, "Caelus", nullptr, nullptr);

    // Initialises vulkan and gives us our instance
    VkInstance instance;
    if (!initVulkan(instance)) return -1;

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        printf("Failed to create window surface!\n");
        return -1;
    }

    VkPhysicalDevice gpu = VK_NULL_HANDLE;
    if (!pickGpu(instance, gpu)) return -1;

    uint32_t queueFamilyIndex = UINT32_MAX;
    if (!findGraphicsQueueFamilyIndex(gpu, queueFamilyIndex, surface)) return -1;

    VkDevice device;
    VkQueue graphicsQueue;
    if (!createLogicalDevice(queueFamilyIndex, gpu, device, graphicsQueue)) return -1;

    SwapChain swapChain;
    if (!createSwapchain(gpu, surface, device, swapChain)) return -1;

    auto vertCode = readFile("shaders/triangle.vert.spv");
    auto fragCode = readFile("shaders/triangle.frag.spv");

    VkShaderModule vertModule = createShaderModule(device, vertCode);
    VkShaderModule fragModule = createShaderModule(device, fragCode);
    printf("Shader modules created\n");

    //! FUCKING PIPELINE SHIT

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
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
    colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingInfo.attachmentCount = 1;
    colorBlendingInfo.pAttachments = &blendAttachment;

    VkDynamicState dynamics[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicInfo{};
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = 2;
    dynamicInfo.pDynamicStates = dynamics;

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPipelineLayout layout;
    vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout);

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &swapChain.format;

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

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        printf("Failed to create graphics pipeline!\n");
        return -1;
    }
    printf("Pipeline created!\n");

    vkDestroyShaderModule(device, vertModule, nullptr);
    vkDestroyShaderModule(device, fragModule, nullptr);

    //! END OF PIPELINE SHIT

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Clean everything up


    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);

    for (VkImageView v : swapChain.views)
        vkDestroyImageView(device, v, nullptr);

    vkDestroySwapchainKHR(device, swapChain.handle, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}