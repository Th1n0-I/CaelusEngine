#include <iostream>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>

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
    // Run twice, same as in pick_gpu()
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, families.data());

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
    printf("succesfully created logical device!\n");
    return true;
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


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Acc delete the window and clean everything up
    glfwDestroyWindow(window);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwTerminate();

    return 0;
}