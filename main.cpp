#include <iostream>
#include <GLFW/glfw3.h>

int main() {

    glfwInit();

    // finds monitors and hooks to the os type shi
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Creates a window >_<
    GLFWwindow* window = glfwCreateWindow(800, 600, "Caelus", nullptr, nullptr);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Acc delete the window and clean everything up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}