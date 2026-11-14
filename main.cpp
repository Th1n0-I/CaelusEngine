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
#include <numbers>
#include <tracy/Tracy.hpp>

#include "Body.h"
#include "Camera.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "Vertex.h"
#include "Math/Math.h"
#include "Mesh.h"
#include "Primitives.h"
#include "KeplerOrbit.h"
#include "SolarSystem.h"
#include "stb_image.h"

using namespace Caelus::Math;

static std::vector<char> readFile(const std::string& path) {
    // Get the file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
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

void drawBodyTree(const std::vector<Caelus::Body>& bodies, int index, int& followIndex, Caelus::Camera& camera) {
    ImGui::PushID(index);

    bool hasChildren = false;
    for (const auto& body : bodies) {
        if (body.parent == index) {
            hasChildren = true;
            break;
        }
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
    if (index == followIndex) flags |= ImGuiTreeNodeFlags_Selected;

    bool open = ImGui::TreeNodeEx(bodies[index].name.c_str(), flags | ImGuiTreeNodeFlags_Framed);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        followIndex = index;
        camera.SetDistance(bodies[index].radius * 5);
    }

    if (open) {
        for (int i = 0; i < bodies.size(); i++) {
            if (bodies[i].parent == index) { drawBodyTree(bodies, i, followIndex, camera); }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
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

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    Caelus::Camera camera{};

    float sensitivity = 0.005f;
    float moveSpeed = 3.0f;
    double lastX, lastY;
    bool looking = false;

    Caelus::MeshData sphereMeshData = Caelus::createCubeSphere(16);

    Caelus::Mesh cubeMesh = renderer.createMesh(sphereMeshData.Vertices, sphereMeshData.Indices);

    auto bodies = Caelus::createSolarSystem();
    
    auto starTexture = renderer.createTexture("C:/Users/volts/Downloads/starmap_2020_16k.png");

    int followIndex = 3;


    double time = 0, timeScale = 1;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        //! Has to after ImGui
        double mouseX, mouseY;
        Vector3 moveVector{.x = 0.0f, .y = 0.0f, .z = 0.0f};

        auto rmbDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        if (rmbDown && !looking && !io.WantCaptureMouse) {
            glfwGetCursorPos(window, &mouseX, &mouseY);
            looking = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            lastX = mouseX;
            lastY = mouseY;
        } else if (rmbDown && looking && !io.WantCaptureMouse) {
            double deltaY, deltaX;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            deltaX = mouseX - lastX;
            deltaY = mouseY - lastY;
            lastX = mouseX;
            lastY = mouseY;
            camera.LookAround(sensitivity * deltaY, sensitivity * -deltaX);
        } else if (!rmbDown && looking && !io.WantCaptureMouse) {
            looking = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { moveVector.y += 1.0; }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { moveVector.y -= 1.0; }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { moveVector.x += 1.0; }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { moveVector.x -= 1.0; }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { moveVector.z += 1.0; }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { moveVector.z -= 1.0; }
        if (moveVector.x != 0.0f || moveVector.y != 0.0f || moveVector.z != 0.0f) {
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
        if (!io.WantCaptureMouse && io.MouseWheel != 0.0f) camera.GetDistance() *= pow(0.9, io.MouseWheel);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Renderer")) {
                if (ImGui::BeginMenu("Clear Color")) {
                    ImGui::ColorPicker3("Clear Color", &clearColor[0]);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Camera")) {
                for (int i = 0; i < bodies.size(); i++) {
                    if (bodies[i].parent == Caelus::Body::noParent) { drawBodyTree(bodies, i, followIndex, camera); }
                }
                ImGui::DragFloat("Sensitivity", &sensitivity, 0.01f);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Simulation")) {
                ImGui::DragScalarN("TimeScale", ImGuiDataType_Double, &timeScale, 1, 0.01f);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        {
            ZoneScopedN("Orbit movement");
            for (auto& body : bodies) {
                if (body.orbit)
                    body.localPosition = Caelus::GetPositionFromOrbit(
                        body.orbit.value(), bodies[body.parent].gravParam.value(), time);
            }

            for (auto& body : bodies) {
                if (body.parent == Caelus::Body::noParent) body.worldPosition = body.localPosition;
                else body.worldPosition = bodies[body.parent].worldPosition + body.localPosition;
            }

            camera.Follow(bodies[followIndex].worldPosition);
        }
        const float aspect = static_cast<float>(renderer.swapChain().extent.width)
            / static_cast<float>(renderer.swapChain().extent.height);
        Matrix4x4 viewProj = camera.GetPerspective(aspect, 0.1f, 100.0f) * camera.GetView();
        {
            ZoneScopedN("Orbit lines");
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            const float W = renderer.swapChain().extent.width;
            const float H = renderer.swapChain().extent.height;

            for (const auto& body : bodies) {
                bool drawPlanetData = true;
                bool drawOrbit = true;

                Vector3 renderPos = toRenderSpace(body.worldPosition, camera.GetPosition());
                Vector4 clipPos = viewProj * Vector4(renderPos.x, renderPos.y, renderPos.z, 1.0f);
                if (clipPos.w <= 0.0f) drawPlanetData = false;

                float screenX = (clipPos.x / clipPos.w * 0.5f + 0.5f) * W;
                float screenY = (clipPos.y / clipPos.w * 0.5f + 0.5f) * H;

                if (body.parent != Caelus::Body::noParent) {
                    Vector3 parentRenderPos = toRenderSpace(bodies[body.parent].worldPosition, camera.GetPosition());
                    Vector4 parentClipPos = viewProj * Vector4(parentRenderPos.x, parentRenderPos.y, parentRenderPos.z,
                                                               1.0f);
                    if (clipPos.w > 0.0f) {
                        float parentScreenX = (parentClipPos.x / parentClipPos.w * 0.5f + 0.5f) * W;
                        float parentScreenY = (parentClipPos.y / parentClipPos.w * 0.5f + 0.5f) * H;

                        if (std::abs(parentScreenX - screenX) < 10 && std::abs(parentScreenY - screenY) < 10) {
                            drawPlanetData = false;
                            drawOrbit = false;
                        }
                    }
                }

                if (body.orbit && drawOrbit) {
                    Vector4 lastClip{};

                    for (int i = 0; i <= body.orbitPoints.size(); i++) {
                        auto position = body.orbitPoints[fmod(i, body.orbitPoints.size())] + bodies[body.parent].
                            worldPosition;
                        Vector3 orbitRenderPos = toRenderSpace(position, camera.GetPosition());
                        Vector4 orbitClipPos = viewProj * Vector4(orbitRenderPos.x, orbitRenderPos.y, orbitRenderPos.z,
                                                                  1.0f);

                        if ((orbitClipPos.w <= 0.0f && lastClip.w <= 0.0f) || i == 0) {
                            lastClip = orbitClipPos;
                            continue;
                        }

                        Vector4 startPos = orbitClipPos;
                        Vector4 endPos = lastClip;

                        lastClip = orbitClipPos;

                        if ((startPos.w - startPos.z) < 0.0f && (endPos.w - endPos.z) < 0.0f) continue;
                        if (startPos.w + startPos.x < 0.0f && endPos.w + endPos.x < 0.0f) continue;
                        if (startPos.w - startPos.x < 0.0f && endPos.w - endPos.x < 0.0f) continue;
                        if (startPos.w + startPos.y < 0.0f && endPos.w + endPos.y < 0.0f) continue;
                        if (startPos.w - startPos.y < 0.0f && endPos.w - endPos.y < 0.0f) continue;

                        if (startPos.w - startPos.z < 0.0f) {
                            float dS = startPos.w - startPos.z;
                            float dE = endPos.w - endPos.z;
                            float t = dS / (dS - dE);
                            startPos = startPos + (endPos - startPos) * t;
                        }
                        if (endPos.w - endPos.z < 0.0f) {
                            float dS = startPos.w - startPos.z;
                            float dE = endPos.w - endPos.z;
                            float t = dE / (dE - dS);
                            endPos = endPos + (startPos - endPos) * t;
                        }
                        if (startPos.w + startPos.x < 0.0f) {
                            float dS = startPos.w + startPos.x;
                            float dE = endPos.w + endPos.x;
                            float t = dS / (dS - dE);
                            startPos = startPos + (endPos - startPos) * t;
                        }
                        if (endPos.w + endPos.x < 0.0f) {
                            float dS = startPos.w + startPos.x;
                            float dE = endPos.w + endPos.x;
                            float t = dE / (dE - dS);
                            endPos = endPos + (startPos - endPos) * t;
                        }
                        if (startPos.w - startPos.x < 0.0f) {
                            float dS = startPos.w - startPos.x;
                            float dE = endPos.w - endPos.x;
                            float t = dS / (dS - dE);
                            startPos = startPos + (endPos - startPos) * t;
                        }
                        if (endPos.w - endPos.x < 0.0f) {
                            float dS = startPos.w - startPos.x;
                            float dE = endPos.w - endPos.x;
                            float t = dE / (dE - dS);
                            endPos = endPos + (startPos - endPos) * t;
                        }
                        if (startPos.w + startPos.y < 0.0f) {
                            float dS = startPos.w + startPos.y;
                            float dE = endPos.w + endPos.y;
                            float t = dS / (dS - dE);
                            startPos = startPos + (endPos - startPos) * t;
                        }
                        if (endPos.w + endPos.y < 0.0f) {
                            float dS = startPos.w + startPos.y;
                            float dE = endPos.w + endPos.y;
                            float t = dE / (dE - dS);
                            endPos = endPos + (startPos - endPos) * t;
                        }
                        if (startPos.w - startPos.y < 0.0f) {
                            float dS = startPos.w - startPos.y;
                            float dE = endPos.w - endPos.y;
                            float t = dS / (dS - dE);
                            startPos = startPos + (endPos - startPos) * t;
                        }
                        if (endPos.w - endPos.y < 0.0f) {
                            float dS = startPos.w - startPos.y;
                            float dE = endPos.w - endPos.y;
                            float t = dE / (dE - dS);
                            endPos = endPos + (startPos - endPos) * t;
                        }

                        float startX = (startPos.x / startPos.w * 0.5f + 0.5f) * W;
                        float startY = (startPos.y / startPos.w * 0.5f + 0.5f) * H;
                        float endX = (endPos.x / endPos.w * 0.5f + 0.5f) * W;
                        float endY = (endPos.y / endPos.w * 0.5f + 0.5f) * H;

                        drawList->AddLine({startX, startY}, {endX, endY}, IM_COL32(255, 255, 255, 100), 1);
                    }
                }

                if (drawPlanetData) {
                    drawList->AddCircle({screenX, screenY}, 6.0f, IM_COL32(80, 150, 255, 255));
                    drawList->AddText({screenX, screenY - 8.0f}, IM_COL32_WHITE, body.name.c_str());
                }
            }
        }

        ImGui::Render();

        VkCommandBuffer cmd = renderer.beginFrame(clearColor);
        if (cmd == VK_NULL_HANDLE) continue;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipelineLayout(), 0, 1, &starTexture.descriptorSet, 0, nullptr);

        Caelus::Consts consts{};
        consts.res[0] = static_cast<int>(renderer.swapChain().extent.width);
        consts.res[1] = static_cast<int>(renderer.swapChain().extent.height);

        const Vector3 sunRenderPos = toRenderSpace(bodies[0].worldPosition, camera.GetPosition());

        {
            ZoneScopedN("Rendering");
            for (const Caelus::Body& body : bodies) {
                const Vector3 renderPos = toRenderSpace(body.worldPosition, camera.GetPosition());
                const float r = body.radius;

                Matrix4x4 model = translate(renderPos) * ::scale(Vector3(r, r, r));
                Matrix4x4 matrix = viewProj * model;

                for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) consts.matrix[i][j] = matrix.m[i][j];

                consts.bodyPosRadius = {renderPos.x, renderPos.y, renderPos.z, body.radius};

                const float emissive = body.emissive ? 1 : 0;
                consts.sunPosEmissive = {sunRenderPos.x, sunRenderPos.y, sunRenderPos.z, emissive};

                vkCmdPushConstants(cmd, renderer.pipelineLayout(),
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Caelus::Consts),
                                   &consts);
                Caelus::Renderer::drawMesh(cmd, cubeMesh);
            }
            Matrix4x4 model = ::scale(Vector3(1e13f, 1e13f, 1e13f));
            Matrix4x4 matrix = viewProj * model;
            
            for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) consts.matrix[i][j] = matrix.m[i][j];
            consts.bodyPosRadius = {0, 0, 0, 1e13f};
            consts.sunPosEmissive.w = 2;
            vkCmdPushConstants(cmd, renderer.pipelineLayout(),
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Caelus::Consts),
                                   &consts);
            Caelus::Renderer::drawMesh(cmd, cubeMesh);
        }

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);


        renderer.endFrame();
        time += io.DeltaTime * timeScale;
        FrameMark;
    }

    vkDeviceWaitIdle(renderer.context().device);

    // Clean everything up

    renderer.destroyTexture(starTexture);
    renderer.destroyMesh(cubeMesh);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
