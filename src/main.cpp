#include <GLFW/glfw3.h>

int window_width  = 800;
int window_height = 800;

int main()
{
    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Deux salles, deux ambiances", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}