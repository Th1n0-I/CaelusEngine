//
// Created by volts on 2026-11-10.
//

#include "VulkanContext.h"
#include <cstdio>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "GLFW/glfw3.h"

namespace Caelus
{
    bool createInstance(VkInstance& instance)
    {
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
        const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
        ci.enabledLayerCount = 1;
        ci.ppEnabledLayerNames = layers;
        #endif


        // Creates the vulkan Instance
        if (vkCreateInstance(&ci, nullptr, &instance) != VK_SUCCESS)
        {
            printf("Failed to create Vulkan instance!\n");
            return false;
        }
        printf("Vulkan instance created!\n");
        return true;
    }

    bool pickGpu(const VkInstance& instance, VkPhysicalDevice& gpu)
    {
        // Finds all gpus
        // call two times, once to get the amount and once to fill an array.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Sorts through devices to find one that is an actual gpu
        for (const auto& device : devices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);
            printf("Device name: %s\n", props.deviceName);

            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                gpu = device;
                return true;
            }
        }

        printf("Failed to find suitable GPU!\n");
        return false;
    }

    //? returns the index of the queue because the queue doesn't exist yet
    bool findGraphicsQueueFamilyIndex(const VkPhysicalDevice& gpu, uint32_t& queueFamilyIndex,
                                      const VkSurfaceKHR& surface)
    {
        // find gpu queue families
        // Run twice, same as in pickGpu()
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, nullptr);

        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, families.data());

        // Find which family can draw and present
        for (uint32_t i = 0; i < familyCount; i++)
        {
            const bool canDraw = families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            VkBool32 canPresent = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &canPresent);
            if (canDraw && canPresent)
            {
                printf("Graphics queue family index: %u\n", i);
                queueFamilyIndex = i;
                return true;
            }
        }
        printf("Could not find graphics queue family index!\n");
        return false;
    }

    bool createLogicalDevice(const uint32_t& queueFamilyIndex, const VkPhysicalDevice& gpu, VkDevice& device,
                             VkQueue& graphicsQueue)
    {
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

        const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        // The "recipe" for the device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &features;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = deviceExtensions;

        // Create the device
        if (vkCreateDevice(gpu, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            printf("Failed to create logical device!\n");
            return false;
        }

        // retrieve the graphicsQueue
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &graphicsQueue);
        printf("successfully created logical device!\n");
        return true;
    }
} // caelus
