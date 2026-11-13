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

#include "Body.h"
#include "Camera.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "Vertex.h"
#include "Math/Math.h"
#include "Mesh.h"
#include "Primitives.h"
#include "KeplerOrbit.h"

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

    // unit helpers: lets us type values exactly as the JPL tables give them
    constexpr double AU = 1.495978707e11; // meters, exact by IAU definition
    constexpr double deg = std::numbers::pi / 180.0; // degrees -> radians
    constexpr double km = 1000.0; // km -> meters
    constexpr double km3 = 1.0e9; // km^3/s^2 -> m^3/s^2

    // ---- index 0
    Caelus::Body sun;
    sun.name = "Sun";
    sun.parent = Caelus::Body::noParent;
    sun.radius = 695700.0 * km;
    sun.emissive = true;
    sun.gravParam = 1.32712440018e11 * km3;

    // ---- index 1
    Caelus::Body mercury;
    mercury.name = "Mercury";
    mercury.parent = 0;
    mercury.radius = 2439.7 * km;
    mercury.gravParam = 22032.0 * km3;
    mercury.orbit = Caelus::KeplerData{
        0.38709927 * AU,
        0.20563593,
        7.00497902 * deg,
        (252.25032350 - 77.45779628) * deg, // M0 = L - long.peri
        (77.45779628 - 48.33076593) * deg, // w  = long.peri - long.node
        48.33076593 * deg
    };

    // ---- index 2
    Caelus::Body venus;
    venus.name = "Venus";
    venus.parent = 0;
    venus.radius = 6051.8 * km;
    venus.gravParam = 324859.0 * km3;
    venus.orbit = Caelus::KeplerData{
        0.72333566 * AU,
        0.00677672,
        3.39467605 * deg,
        (181.97909950 - 131.60246718) * deg,
        (131.60246718 - 76.67984255) * deg,
        76.67984255 * deg
    };

    // ---- index 3
    Caelus::Body earth;
    earth.name = "Earth";
    earth.parent = 0;
    earth.radius = 6371.0 * km;
    earth.gravParam = 398600.4418 * km3;
    earth.orbit = Caelus::KeplerData{
        1.00000261 * AU,
        0.01671123,
        -0.00001531 * deg,
        (100.46457166 - 102.93768193) * deg,
        (102.93768193 - 0.0) * deg,
        0.0 * deg
    };

    // ---- index 5
    Caelus::Body mars;
    mars.name = "Mars";
    mars.parent = 0;
    mars.radius = 3389.5 * km;
    mars.gravParam = 42828.37 * km3;
    mars.orbit = Caelus::KeplerData{
        1.52371034 * AU,
        0.09339410,
        1.84969142 * deg,
        (-4.55343205 - -23.94362959) * deg,
        (-23.94362959 - 49.55953891) * deg,
        49.55953891 * deg
    };

    // ---- index 8
    Caelus::Body jupiter;
    jupiter.name = "Jupiter";
    jupiter.parent = 0;
    jupiter.radius = 69911.0 * km;
    jupiter.gravParam = 126686534.0 * km3;
    jupiter.orbit = Caelus::KeplerData{
        5.20288700 * AU,
        0.04838624,
        1.30439695 * deg,
        (34.39644051 - 14.72847983) * deg,
        (14.72847983 - 100.47390909) * deg,
        100.47390909 * deg
    };

    // ---- index 13
    Caelus::Body saturn;
    saturn.name = "Saturn";
    saturn.parent = 0;
    saturn.radius = 58232.0 * km;
    saturn.gravParam = 37931187.0 * km3;
    saturn.orbit = Caelus::KeplerData{
        9.53667594 * AU,
        0.05386179,
        2.48599187 * deg,
        (49.95424423 - 92.59887831) * deg,
        (92.59887831 - 113.66242448) * deg,
        113.66242448 * deg
    };

    // ---- index 21
    Caelus::Body uranus;
    uranus.name = "Uranus";
    uranus.parent = 0;
    uranus.radius = 25362.0 * km;
    uranus.gravParam = 5793939.0 * km3;
    uranus.orbit = Caelus::KeplerData{
        19.18916464 * AU,
        0.04725744,
        0.77263783 * deg,
        (313.23810451 - 170.95427630) * deg,
        (170.95427630 - 74.01692503) * deg,
        74.01692503 * deg
    };

    // ---- index 27
    Caelus::Body neptune;
    neptune.name = "Neptune";
    neptune.parent = 0;
    neptune.radius = 24622.0 * km;
    neptune.gravParam = 6836529.0 * km3;
    neptune.orbit = Caelus::KeplerData{
        30.06992276 * AU,
        0.00859048,
        1.77004347 * deg,
        (-55.12002969 - 44.96476227) * deg,
        (44.96476227 - 131.78422574) * deg,
        131.78422574 * deg
    };

    // ---- index 4
    Caelus::Body moon;
    moon.name = "Moon";
    moon.parent = 3;
    moon.radius = 1737.4 * km;
    moon.gravParam = 4902.8 * km3;
    moon.orbit = Caelus::KeplerData{384400.0 * km, 0.0549, 5.145 * deg, 0.0, 0.0, 0.0};

    // ---- index 6
    Caelus::Body phobos;
    phobos.name = "Phobos";
    phobos.parent = 5;
    phobos.radius = 11.1 * km;
    phobos.gravParam = 0.000709 * km3;
    phobos.orbit = Caelus::KeplerData{9376.0 * km, 0.0151, 1.08 * deg, 0.0, 0.0, 0.0};

    // ---- index 7
    Caelus::Body deimos;
    deimos.name = "Deimos";
    deimos.parent = 5;
    deimos.radius = 6.2 * km;
    deimos.gravParam = 0.000096 * km3;
    deimos.orbit = Caelus::KeplerData{23463.0 * km, 0.0003, 1.79 * deg, 2.0, 0.0, 0.0};

    // ---- index 9
    Caelus::Body ioMoon;
    ioMoon.name = "Io";
    ioMoon.parent = 8;
    ioMoon.radius = 1821.6 * km;
    ioMoon.gravParam = 5959.9 * km3;
    ioMoon.orbit = Caelus::KeplerData{421800.0 * km, 0.0041, 0.04 * deg, 0.0, 0.0, 0.0};

    // ---- index 10
    Caelus::Body europa;
    europa.name = "Europa";
    europa.parent = 8;
    europa.radius = 1560.8 * km;
    europa.gravParam = 3202.7 * km3;
    europa.orbit = Caelus::KeplerData{671100.0 * km, 0.0094, 0.47 * deg, 1.6, 0.0, 0.0};

    // ---- index 11
    Caelus::Body ganymede;
    ganymede.name = "Ganymede";
    ganymede.parent = 8;
    ganymede.radius = 2631.2 * km;
    ganymede.gravParam = 9887.8 * km3;
    ganymede.orbit = Caelus::KeplerData{1070400.0 * km, 0.0013, 0.18 * deg, 3.1, 0.0, 0.0};

    // ---- index 12
    Caelus::Body callisto;
    callisto.name = "Callisto";
    callisto.parent = 8;
    callisto.radius = 2410.3 * km;
    callisto.gravParam = 7179.3 * km3;
    callisto.orbit = Caelus::KeplerData{1882700.0 * km, 0.0074, 0.19 * deg, 4.7, 0.0, 0.0};

    // ---- index 14
    Caelus::Body mimas;
    mimas.name = "Mimas";
    mimas.parent = 13;
    mimas.radius = 198.2 * km;
    mimas.gravParam = 2.50 * km3;
    mimas.orbit = Caelus::KeplerData{185540.0 * km, 0.0196, 1.57 * deg, 0.0, 0.0, 0.0};

    // ---- index 15
    Caelus::Body enceladus;
    enceladus.name = "Enceladus";
    enceladus.parent = 13;
    enceladus.radius = 252.1 * km;
    enceladus.gravParam = 7.21 * km3;
    enceladus.orbit = Caelus::KeplerData{238040.0 * km, 0.0047, 0.01 * deg, 0.9, 0.0, 0.0};

    // ---- index 16
    Caelus::Body tethys;
    tethys.name = "Tethys";
    tethys.parent = 13;
    tethys.radius = 531.1 * km;
    tethys.gravParam = 41.2 * km3;
    tethys.orbit = Caelus::KeplerData{294670.0 * km, 0.0001, 1.09 * deg, 1.8, 0.0, 0.0};

    // ---- index 17
    Caelus::Body dione;
    dione.name = "Dione";
    dione.parent = 13;
    dione.radius = 561.4 * km;
    dione.gravParam = 73.1 * km3;
    dione.orbit = Caelus::KeplerData{377420.0 * km, 0.0022, 0.03 * deg, 2.7, 0.0, 0.0};

    // ---- index 18
    Caelus::Body rhea;
    rhea.name = "Rhea";
    rhea.parent = 13;
    rhea.radius = 763.8 * km;
    rhea.gravParam = 153.9 * km3;
    rhea.orbit = Caelus::KeplerData{527070.0 * km, 0.0010, 0.35 * deg, 3.6, 0.0, 0.0};

    // ---- index 19
    Caelus::Body titan;
    titan.name = "Titan";
    titan.parent = 13;
    titan.radius = 2574.7 * km;
    titan.gravParam = 8978.1 * km3;
    titan.orbit = Caelus::KeplerData{1221870.0 * km, 0.0288, 0.31 * deg, 4.5, 0.0, 0.0};

    // ---- index 20
    Caelus::Body iapetus;
    iapetus.name = "Iapetus";
    iapetus.parent = 13;
    iapetus.radius = 734.5 * km;
    iapetus.gravParam = 120.5 * km3;
    iapetus.orbit = Caelus::KeplerData{3560840.0 * km, 0.0283, 15.5 * deg, 5.4, 0.0, 0.0};

    // ---- index 22
    Caelus::Body miranda;
    miranda.name = "Miranda";
    miranda.parent = 21;
    miranda.radius = 235.8 * km;
    miranda.gravParam = 4.4 * km3;
    miranda.orbit = Caelus::KeplerData{129900.0 * km, 0.0013, 4.34 * deg, 0.0, 0.0, 0.0};

    // ---- index 23
    Caelus::Body ariel;
    ariel.name = "Ariel";
    ariel.parent = 21;
    ariel.radius = 578.9 * km;
    ariel.gravParam = 83.5 * km3;
    ariel.orbit = Caelus::KeplerData{190900.0 * km, 0.0012, 0.04 * deg, 1.3, 0.0, 0.0};

    // ---- index 24
    Caelus::Body umbriel;
    umbriel.name = "Umbriel";
    umbriel.parent = 21;
    umbriel.radius = 584.7 * km;
    umbriel.gravParam = 85.1 * km3;
    umbriel.orbit = Caelus::KeplerData{266000.0 * km, 0.0039, 0.13 * deg, 2.5, 0.0, 0.0};

    // ---- index 25
    Caelus::Body titania;
    titania.name = "Titania";
    titania.parent = 21;
    titania.radius = 788.9 * km;
    titania.gravParam = 226.9 * km3;
    titania.orbit = Caelus::KeplerData{436300.0 * km, 0.0011, 0.08 * deg, 3.8, 0.0, 0.0};

    // ---- index 26
    Caelus::Body oberon;
    oberon.name = "Oberon";
    oberon.parent = 21;
    oberon.radius = 761.4 * km;
    oberon.gravParam = 205.3 * km3;
    oberon.orbit = Caelus::KeplerData{583500.0 * km, 0.0014, 0.07 * deg, 5.0, 0.0, 0.0};

    // ---- index 28
    Caelus::Body triton;
    triton.name = "Triton";
    triton.parent = 27;
    triton.radius = 1353.4 * km;
    triton.gravParam = 1428.5 * km3;
    triton.orbit = Caelus::KeplerData{354759.0 * km, 0.000016, 156.865 * deg, 0.0, 0.0, 0.0}; // >90 deg = retrograde

    std::vector<Caelus::Body> bodies{
        sun, mercury, venus, earth, moon, mars, phobos, deimos,
        jupiter, ioMoon, europa, ganymede, callisto,
        saturn, mimas, enceladus, tethys, dione, rhea, titan, iapetus,
        uranus, miranda, ariel, umbriel, titania, oberon,
        neptune, triton
    };
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
                if (ImGui::BeginMenu("Target")) {
                    for (int i = 0; i < bodies.size(); i++)
                        if (ImGui::Button(bodies[i].name.c_str())) {
                            followIndex = i;
                            camera.SetDistance(bodies[followIndex].radius * 5);
                        }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::Begin("Caelus");
        ImGui::Text("%.1f FPS", io.Framerate);

        for (int i = 0; i < bodies.size(); i++) {
            ImGui::PushID(i);
            if (ImGui::CollapsingHeader(bodies[i].name.c_str())) {
                ImGui::DragScalarN(bodies[i].name.c_str(), ImGuiDataType_Double, &bodies[i].localPosition.x, 3, 0.01f);
                ImGui::DragFloat("Scale", &bodies[i].radius, 0.01f);
            }
            ImGui::PopID();
        }

        if (ImGui::CollapsingHeader("Camera")) {
            ImGui::DragScalarN("Position", ImGuiDataType_Double, &camera.GetPosition().x, 3, 0.01f);
            ImGui::DragFloat("Pitch", &camera.GetPitch(), 0.01f);
            ImGui::DragFloat("Yaw", &camera.GetYaw(), 0.01f);
            ImGui::DragFloat("Sensitivity", &sensitivity, 0.01f);
        }
        ImGui::DragScalarN("TimeScale", ImGuiDataType_Double, &timeScale, 1, 0.01f);
        ImGui::Separator();

        ImGui::End();

        for (auto& body : bodies) {
            if (body.orbit) body.localPosition = Caelus::GetPositionFromOrbit(
                body.orbit.value(), bodies[body.parent].gravParam.value(), time);
        }

        for (auto& body : bodies) {
            if (body.parent == Caelus::Body::noParent) body.worldPosition = body.localPosition;
            else body.worldPosition = bodies[body.parent].worldPosition + body.localPosition;
        }

        camera.Follow(bodies[followIndex].worldPosition);

        const float aspect = static_cast<float>(renderer.swapChain().extent.width)
            / static_cast<float>(renderer.swapChain().extent.height);

        Matrix4x4 viewProj = camera.GetPerspective(aspect, 0.1f, 100.0f) * camera.GetView();

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
                auto& orbit = body.orbit.value();
                constexpr int orbitPoints = 128;
                
                Vector4 lastClip{};

                for (int i = 0; i <= orbitPoints; i++) {
                    double orbitPeriod = std::numbers::pi * 2 * sqrt(
                        orbit.semiMajorAxis * orbit.semiMajorAxis * orbit.semiMajorAxis / bodies[body.parent].gravParam.
                        value());
                    auto point = time + i * orbitPeriod / orbitPoints;
                    auto position = Caelus::GetPositionFromOrbit(orbit, bodies[body.parent].gravParam.value(), point);
                    position = bodies[body.parent].worldPosition + position;
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
                    if (startPos.w + startPos.x < 0.0f && endPos.w + endPos.x < 0.0f ) continue;
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

        ImGui::Render();

        VkCommandBuffer cmd = renderer.beginFrame(clearColor);
        if (cmd == VK_NULL_HANDLE) continue;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline());

        Caelus::Consts consts{};
        consts.res[0] = static_cast<int>(renderer.swapChain().extent.width);
        consts.res[1] = static_cast<int>(renderer.swapChain().extent.height);

        const Vector3 sunRenderPos = toRenderSpace(bodies[0].worldPosition, camera.GetPosition());

        for (const Caelus::Body& body : bodies) {
            const Vector3 renderPos = toRenderSpace(body.worldPosition, camera.GetPosition());
            const float r = body.radius;

            Matrix4x4 model = translate(renderPos) * ::scale(Vector3(r, r, r));
            Matrix4x4 matrix = viewProj * model;

            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++) consts.matrix[i][j] = matrix.m[i][j];

            consts.bodyPosRadius = {renderPos.x, renderPos.y, renderPos.z, body.radius};

            const float emissive = body.emissive ? 1 : 0;
            consts.sunPosEmissive = {sunRenderPos.x, sunRenderPos.y, sunRenderPos.z, emissive};

            vkCmdPushConstants(cmd, renderer.pipelineLayout(),
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Caelus::Consts),
                               &consts);
            Caelus::Renderer::drawMesh(cmd, cubeMesh);
        }

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);


        renderer.endFrame();
        time += io.DeltaTime * timeScale;
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
