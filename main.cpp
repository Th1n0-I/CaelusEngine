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

#include "Camera.h"
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
    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

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
    WorldPos position = { .x = 0.0f, .y = 0.0f, .z = -3.0f };
    float rotation[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3] = { 1.0f, 1.0f, 1.0f };

    Caelus::Camera camera{};



    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Renderer")) {
                if (ImGui::BeginMenu("Clear Color")) {
                    ImGui::ColorPicker3("Clear Color", &clearColor[0]);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::Begin("Caelus");
        ImGui::Text("%.1f FPS", io.Framerate);
        if (ImGui::CollapsingHeader("Triangle")){
            ImGui::DragScalarN("Position", ImGuiDataType_Double, &position.x, 3, 0.01f);
            ImGui::DragFloat3("Rotation", &rotation[0], 0.01f);
            ImGui::DragFloat3("Scale", &scale[0], 0.01f);
        }
        if (ImGui::CollapsingHeader("Camera")) {
            ImGui::DragScalarN("Position", ImGuiDataType_Double, &camera.GetPosition().x, 3, 0.01f);
            ImGui::DragFloat("Pitch", &camera.GetPitch(), 0.01f);
            ImGui::DragFloat("Yaw", &camera.GetYaw(), 0.01f);
        }
        ImGui::Separator();

        ImGui::End();

        ImGui::Render();

        const float aspect = static_cast<float>(renderer.swapChain().extent.width)
                       / static_cast<float>(renderer.swapChain().extent.height);

        const Vector3 renderPos = toRenderSpace(position, camera.GetPosition());

        Matrix4x4 viewProj =
            camera.GetPerspective(aspect, 0.1f, 100.0f) *
                        camera.GetView();
        Matrix4x4 model =
            translate(renderPos)  *
                        rotateZ(rotation[2]) *
                        rotateY(rotation[1]) *
                        rotateX(rotation[0]) *
                        ::scale(Vector3(scale[0], scale[1], scale[2]));
        auto matrix = viewProj * model;


        VkCommandBuffer cmd = renderer.beginFrame(clearColor);
        if (cmd == VK_NULL_HANDLE) continue;

        Caelus::Consts consts{};
        consts.res[0] = static_cast<int>(renderer.swapChain().extent.width);
        consts.res[1] = static_cast<int>(renderer.swapChain().extent.height);

        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                consts.matrix[i][j] = matrix.m[i][j];

        vkCmdPushConstants(cmd, renderer.pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Caelus::Consts), &consts);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline());
        vkCmdDraw(cmd, 36, 1, 0, 0);
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