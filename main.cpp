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
#include "Pipeline.h"
#include "Renderer.h"
#include "Vertex.h"
#include "Math/Math.h"
#include "Mesh.h"

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

    const int cubeRes = 8;
    const int R = cubeRes + 1;

    std::vector<Caelus::Vertex> cubeVerts;
    std::vector<int> indexSpots(R * R * R, -1);

    for (int x = 0; x <= cubeRes; x++)
    {
        float posX = static_cast<float>(x) / static_cast<float>(cubeRes) - 0.5f;
        bool xOnEdge = x == 0 || x == cubeRes;
        for (int y = 0; y <= cubeRes; y++)
        {
            float posY = static_cast<float>(y) / static_cast<float>(cubeRes) - 0.5f;
            bool yOnEdge = y == 0 || y == cubeRes;
            for (int z = 0; z <= cubeRes; z++)
            {
                float posZ = static_cast<float>(z) / static_cast<float>(cubeRes) - 0.5f;
                bool zOnEdge = z == 0 || z == cubeRes;
                if (!xOnEdge && !yOnEdge && !zOnEdge) continue;
                int mapIndex = (x * R + y) * R + z;
                indexSpots[mapIndex] = cubeVerts.size();
                cubeVerts.push_back({normalize(Vector3{posX, posY, posZ})});
            }
        }
    }

    std::vector<uint32_t> cubeIndices;
    cubeIndices.reserve(36 * cubeRes * cubeRes);

    auto vert = [&](int x, int y, int z)
    {
        return static_cast<uint32_t>(indexSpots[( x * R + y ) * R + z]);
    };

    auto addQuad = [&](uint32_t a, uint32_t b, uint32_t c, uint32_t d, bool flip)
    {
        if (!flip) cubeIndices.insert(cubeIndices.end(), {a, b ,c , a, c,d});
        else cubeIndices.insert(cubeIndices.end(), {a,c,b,a,d,c});
    };

    for (int i = 0; i < cubeRes; i++)
    {
        for (int j = 0; j < cubeRes; j++)
        {
            for (int side = 0; side <= cubeRes; side += cubeRes)
            {
                bool flip = (side == cubeRes);
                addQuad(vert(side, i, j), vert(side, i + 1, j), vert(side, i + 1, j + 1), vert(side, i, j + 1), flip);
                addQuad(vert(j, side, i), vert(j, side, i + 1), vert(j + 1, side, i + 1), vert(j + 1, side, i), flip);
                addQuad(vert(i, j, side), vert(i + 1, j, side), vert(i + 1, j + 1, side), vert(i, j + 1, side), flip);
            }
        }
    }

    Caelus::Mesh cubeMesh = renderer.createMesh(cubeVerts, cubeIndices);

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
        if (ImGui::CollapsingHeader("Triangle"))
        {
            ImGui::DragScalarN("Position", ImGuiDataType_Double, &position.x, 3, 0.01f);
            ImGui::DragFloat3("Rotation", &rotation[0], 0.01f);
            ImGui::DragFloat3("Scale", &scale[0], 0.01f);
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

        const float aspect = static_cast<float>(renderer.swapChain().extent.width)
            / static_cast<float>(renderer.swapChain().extent.height);

        const Vector3 renderPos = toRenderSpace(position, camera.GetPosition());

        Matrix4x4 viewProj =
            camera.GetPerspective(aspect, 0.1f, 100.0f) *
            camera.GetView();
        Matrix4x4 model =
            translate(renderPos) *
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

        vkCmdPushConstants(cmd, renderer.pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Caelus::Consts),
                           &consts);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline());
        VkDeviceSize offset = 0;
        Caelus::Renderer::drawMesh(renderer.commandBuffer(), cubeMesh);
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
