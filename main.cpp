#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <fstream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Renderer.h"
#include "Math/Math.h"

using namespace Caelus::Math;

static std::vector<char> readFile(const std::string &path) {
    // Get the file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        printf("Failed to open %s\n", path.c_str());
        return {};
    }

    // Get the size of the file
    const size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    // Return the file contents
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    return buffer;
}

int main() {

    glfwInit();

    // finds monitors and hooks to the os type shi
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto vertCode = readFile("shaders/triangle.vert.spv");
    auto fragCode = readFile("shaders/triangle.frag.spv");

    // Creates a window >_<
    GLFWwindow* window = glfwCreateWindow(800, 600, "Caelus", nullptr, nullptr);
    Caelus::Renderer renderer(window, vertCode, fragCode);

    // The context, whatever that is
    ImGui::CreateContext();

    // The input / output, you tell it what inputs ur doing and it tells you if those inputs are meant for ImGui
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Lets windows snap to each other
    ImGui::StyleColorsDark(); // Dark mode cuz i don wanna get flashbanged

    // Links imgui with glfw
    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &renderer.swapChain().format;

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_3;
    initInfo.Instance = renderer.context().instance;
    initInfo.PhysicalDevice = renderer.context().gpu;
    initInfo.Device = renderer.context().device;
    initInfo.QueueFamily = renderer.context().queueFamilyIndex;
    initInfo.Queue = renderer.context().graphicsQueue;
    initInfo.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(renderer.swapChain().images.size());
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = renderingInfo;

    ImGui_ImplVulkan_Init(&initInfo);
    printf("Successfully initialized ImGui!\n");

    float clearColor[4] = { 0.02f, 0.02f, 0.05f, 1.0f };
    float positions[6] = {
         0.0, -0.5,
         0.5,  0.5,
        -0.5,  0.5
    };

    Matrix4x4 t = identity(); t.m[3][0] = 10;
    Matrix4x4 s = identity(); s.m[0][0] = 2;

    auto v = Vector4{.x = 1, .y = 3, .z = 2, .w = 1};
    v = (t * s) * v;
    printf("(%f, %f, %f, %f)", v.x, v.y, v.z, v.w);


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Caelus");
        ImGui::Text("%.1f FPS", io.Framerate);
        ImGui::ColorPicker3("Clear color", &clearColor[0]);
        ImGui::DragFloat2("vert1Pos", &positions[0], 0.01f);
        ImGui::DragFloat2("vert2Pos", &positions[2], 0.01f);
        ImGui::DragFloat2("vert3Pos", &positions[4], 0.01f);

        ImGui::End();

        ImGui::Render();

        VkCommandBuffer cmd = renderer.beginFrame(clearColor);

        Caelus::Consts consts{};
        consts.res[0] = static_cast<int>(renderer.swapChain().extent.width);
        consts.res[1] = static_cast<int>(renderer.swapChain().extent.height);

        for (int i = 0; i < 6; i++)
        {
            consts.pos[i] = positions[i];
        }

        vkCmdPushConstants(cmd, renderer.pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Caelus::Consts), &consts);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline());
        vkCmdDraw(cmd, 3, 1, 0, 0);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);


        renderer.endFrame();
    }

    vkDeviceWaitIdle(renderer.context().device);

    // Clean everything up

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}