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

#include "Body.h"
#include "Camera.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "Vertex.h"
#include "Math/Math.h"
#include "Mesh.h"
#include "Primitives.h"

using namespace Caelus::Math;

static std::vector<char> readFile(const std::string& path)
{
    // Get the file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        printf("Failed to open %s\n", path.c_str());
        return {};
    }

    // Get the size of the file
    const size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    // Return the file contents
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    return buffer;
}

int main()
{
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
    ImGuiIO& io = ImGui::GetIO();
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

    float clearColor[4] = {0.02f, 0.02f, 0.05f, 1.0f};
    WorldPos position = {.x = 0.0f, .y = 0.0f, .z = -3.0f};
    float rotation[3] = {0.0f, 0.0f, 0.0f};
    float scale[3] = {1.0f, 1.0f, 1.0f};

    Caelus::Camera camera{};

    float sensitivity = 0.005f;
    float moveSpeed = 3.0f;
    double lastX, lastY;
    bool looking = false;

    Caelus::MeshData sphereMeshData = Caelus::createCubeSphere(16);

    Caelus::Mesh cubeMesh = renderer.createMesh(sphereMeshData.Vertices, sphereMeshData.Indices);
    
    Caelus::Body sun;
    sun.name = "Sun";
    sun.localPosition = WorldPos(0, 0, 0);
    sun.parent = Caelus::Body::noParent;
    sun.radius = 2;
    sun.emissive = true;
    
    Caelus::Body earth;
    earth.name = "Earth";
    earth.localPosition = WorldPos(10, 0, 0);
    earth.parent = 0;
    earth.radius = 1;
    
    Caelus::Body moon;
    moon.name = "Moon";
    moon.localPosition = WorldPos(3, 0, 0);
    moon.parent = 1;
    moon.radius = 0.27f;
    
    std::vector bodies {sun, earth, moon};

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();


        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        //! Has to after ImGui
        double mouseX, mouseY;
        Vector3 moveVector{.x = 0.0f, .y = 0.0f, .z = 0.0f};

        auto rmbDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        if (rmbDown && !looking && !io.WantCaptureMouse)
        {
            glfwGetCursorPos(window, &mouseX, &mouseY);
            looking = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            lastX = mouseX;
            lastY = mouseY;
        }
        else if (rmbDown && looking && !io.WantCaptureMouse)
        {
            double deltaY, deltaX;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            deltaX = mouseX - lastX;
            deltaY = mouseY - lastY;
            lastX = mouseX;
            lastY = mouseY;
            camera.LookAround(sensitivity * deltaY, sensitivity * -deltaX);
        }
        else if (!rmbDown && looking && !io.WantCaptureMouse)
        {
            looking = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { moveVector.y += 1.0; }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { moveVector.y -= 1.0; }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { moveVector.x += 1.0; }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { moveVector.x -= 1.0; }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { moveVector.z += 1.0; }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { moveVector.z -= 1.0; }
        if (moveVector.x != 0.0f || moveVector.y != 0.0f || moveVector.z != 0.0f)
        {
            moveVector = normalize(moveVector) * moveSpeed * io.DeltaTime;
            Matrix4x4 v = camera.GetView();
            Vector3 right = {v.m[0][0], v.m[1][0], v.m[2][0]};
            Vector3 forward = {-v.m[0][2], -v.m[1][2], -v.m[2][2]};
            Vector3 up = {0.0f, 1.0f, 0.0f};
            right = right * moveVector.x;
            forward = forward * moveVector.y;
            up = up * moveVector.z;
            moveVector = up + right + forward;

            camera.GetPosition() = {
                .x = camera.GetPosition().x + moveVector.x,
                .y = camera.GetPosition().y + moveVector.y,
                .z = camera.GetPosition().z + moveVector.z
            };
        }


        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Renderer"))
            {
                if (ImGui::BeginMenu("Clear Color"))
                {
                    ImGui::ColorPicker3("Clear Color", &clearColor[0]);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::Begin("Caelus");
        ImGui::Text("%.1f FPS", io.Framerate);
        
        for (int i = 0; i < bodies.size(); i++)
        {
            ImGui::PushID(i);
            if (ImGui::CollapsingHeader(bodies[i].name.c_str()))
            {
                ImGui::DragScalarN(bodies[i].name.c_str(), ImGuiDataType_Double, &bodies[i].localPosition.x, 3, 0.01f);
                ImGui::DragFloat("Scale", &bodies[i].radius, 0.01f);
            }
            ImGui::PopID();
        }
        
        if (ImGui::CollapsingHeader("Camera"))
        {
            ImGui::DragScalarN("Position", ImGuiDataType_Double, &camera.GetPosition().x, 3, 0.01f);
            ImGui::DragFloat("Pitch", &camera.GetPitch(), 0.01f);
            ImGui::DragFloat("Yaw", &camera.GetYaw(), 0.01f);
            ImGui::DragFloat("Sensitivity", &sensitivity, 0.01f);
        }
        ImGui::Separator();

        ImGui::End();

        ImGui::Render();
        
        for (Caelus::Body& body : bodies)
        {
            if (body.parent == Caelus::Body::noParent) body.worldPosition = body.localPosition;
            else body.worldPosition = bodies[body.parent].worldPosition + body.localPosition;
        }
        

        const float aspect = static_cast<float>(renderer.swapChain().extent.width)
            / static_cast<float>(renderer.swapChain().extent.height);

        Matrix4x4 viewProj = camera.GetPerspective(aspect, 0.1f, 100.0f) * camera.GetView();
        
        VkCommandBuffer cmd = renderer.beginFrame(clearColor);
        if (cmd == VK_NULL_HANDLE) continue;
        
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline());
        
        Caelus::Consts consts{};
        consts.res[0] = static_cast<int>(renderer.swapChain().extent.width);
        consts.res[1] = static_cast<int>(renderer.swapChain().extent.height);
        
        const Vector3 sunRenderPos = toRenderSpace(bodies[0].worldPosition, camera.GetPosition());
        
        for (const Caelus::Body& body : bodies)
        {
            const Vector3 renderPos = toRenderSpace(body.worldPosition, camera.GetPosition());
            const float r = body.radius;
            
            Matrix4x4 model = translate(renderPos) * ::scale(Vector3(r, r, r));
            Matrix4x4 matrix = viewProj * model;
            
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    consts.matrix[i][j] = matrix.m[i][j];
            
            consts.bodyPosRadius = {renderPos.x, renderPos.y, renderPos.z, body.radius};
            
            const float emissive = body.emissive ? 1 : 0;
            consts.sunPosEmissive = {sunRenderPos.x, sunRenderPos.y, sunRenderPos.z, emissive};
            
            vkCmdPushConstants(cmd, renderer.pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Caelus::Consts), &consts);
            Caelus::Renderer::drawMesh(cmd, cubeMesh);
        }
        
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);


        renderer.endFrame();
    }

    vkDeviceWaitIdle(renderer.context().device);

    // Clean everything up

    renderer.destroyMesh(cubeMesh);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
