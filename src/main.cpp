#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glimac/FilePath.hpp>
#include <glimac/Program.hpp>
#include <glimac/FreeflyCamera.hpp>
#include <glimac/Sphere.hpp>
#include <cstddef>

using namespace glimac;

int window_width = 800;
int window_height = 800;

const GLuint VERTEX_ATTR_POSITION = 0;
const GLuint VERTEX_ATTR_NORMAL = 1;
const GLuint VERTEX_ATTR_COLOR = 2;

bool move = false;
FreeflyCamera camera;
float cameraHeight = 0.f;

struct Vertex3DColor
{
    glm::vec3 position;
    glm::vec3 color;

    Vertex3DColor(const glm::vec3 &position, const glm::vec3 &color)
        : position(position), color(color)
    {
    }
};

static void key_callback(GLFWwindow * /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{

    if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_W)
    {
        camera.moveFront(0.1);
        // camera.setCameraPositionY(cameraHeight);
    }
    else if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_S)
    {
        camera.moveFront(-0.1);
        // camera.setCameraPositionY(cameraHeight);
    }
    else if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_A)
    {
        camera.moveLeft(0.1);
        // camera.setCameraPositionY(cameraHeight);
    }
    else if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_D)
    {
        camera.moveLeft(-0.1);
        // camera.setCameraPositionY(cameraHeight);
    }
}

static void mouse_button_callback(GLFWwindow * /*window*/, int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        move = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        move = false;
    }
}

static void cursor_position_callback(GLFWwindow * /*window*/, double xpos, double ypos)
{
    static double lastX = xpos;
    static double lastY = ypos;

    if (move)
    {
        camera.rotateLeft((xpos - lastX));
        camera.rotateUp((ypos - lastY));
    }

    lastX = xpos;
    lastY = ypos;
}

static void size_callback(GLFWwindow * /*window*/, int width, int height)
{
    window_width = width;
    window_height = height;
}

int main(int /*argc*/, char **argv)
{
    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "Deux salles, deux ambiances", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return -1;
    }

    FilePath applicationPath(argv[0]);
    Program sphereProgram = loadProgram(applicationPath.dirPath() + "../src/shaders/3D.vs.glsl",
                                        applicationPath.dirPath() + "../src/shaders/normals.fs.glsl");
    sphereProgram.use();

    // Get uniform location for sphere
    GLint sphereMVPMatrixLocation = glGetUniformLocation(sphereProgram.getGLId(), "uMVPMatrix");
    GLint sphereMVMatrixLocation = glGetUniformLocation(sphereProgram.getGLId(), "uMVMatrix");
    GLint sphereNormalMatrixLocation = glGetUniformLocation(sphereProgram.getGLId(), "uNormalMatrix");

    Program squareProgram = loadProgram(applicationPath.dirPath() + "../src/shaders/3D.vs.glsl",
                                        applicationPath.dirPath() + "../src/shaders/color.fs.glsl");
    squareProgram.use();

    // Get uniform location for square
    GLint squareMVPMatrixLocation = glGetUniformLocation(squareProgram.getGLId(), "uMVPMatrix");
    GLint squareMVMatrixLocation = glGetUniformLocation(squareProgram.getGLId(), "uMVMatrix");
    GLint squareNormalMatrixLocation = glGetUniformLocation(squareProgram.getGLId(), "uNormalMatrix");

    glEnable(GL_DEPTH_TEST);

    /* Hook input callbacks */
    glfwSetKeyCallback(window, &key_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetCursorPosCallback(window, &cursor_position_callback);
    glfwSetWindowSizeCallback(window, &size_callback);

    /***********************
     * INITIALIZATION CODE
     ***********************/

    /**********
     * SPHERE
     **********/

    Sphere sphere(1, 32, 16);

    /* VBO */
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sphere.getVertexCount() * sizeof(ShapeVertex), sphere.getDataPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* VAO */
    GLuint vao;
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), (const GLvoid *)offsetof(ShapeVertex, position));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), (const GLvoid *)offsetof(ShapeVertex, normal));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    /*********
     * FLOOR
     *********/

    Vertex3DColor floorVertices[] = {
        Vertex3DColor(glm::vec3(-12.f, -21.f, 0.f), glm::vec3(0.4f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(12.f, -21.f, 0.f), glm::vec3(0.4f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(12.f, 21.f, 0.f), glm::vec3(0.4f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(-12.f, 21.f, 0.f), glm::vec3(0.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(-12.f, -21.f, 0.f), glm::vec3(0.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(12.f, 21.f, 0.f), glm::vec3(0.f, 0.25f, 0.2f))};

    /* VBO & VAO */
    GLuint floorVBO, floorVAO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, position));

    glEnableVertexAttribArray(VERTEX_ATTR_COLOR);
    glVertexAttribPointer(VERTEX_ATTR_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /********
     * WALLS
     ********/

    Vertex3DColor wallVertices[] = {
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(12.f, -3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(-12.f, 3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f)),
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f))};

    /* VBO & VAO */
    GLuint wallVBO, wallVAO;
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);

    glBindVertexArray(wallVAO);

    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)offsetof(Vertex3DColor, position));

    glEnableVertexAttribArray(VERTEX_ATTR_COLOR);
    glVertexAttribPointer(VERTEX_ATTR_COLOR, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)offsetof(Vertex3DColor, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /**************
     * SMALL WALLS
     **************/
    Vertex3DColor smallWallVertices[] = {
        Vertex3DColor(glm::vec3(-5.f, -3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(5.f, -3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(5.f, 3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(-5.f, 3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f)),
        Vertex3DColor(glm::vec3(-5.f, -3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f)),
        Vertex3DColor(glm::vec3(5.f, 3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f))};

    /* VBO & VAO */
    GLuint smallWallVBO, smallWallVAO;
    glGenVertexArrays(1, &smallWallVAO);
    glGenBuffers(1, &smallWallVBO);

    glBindVertexArray(smallWallVAO);

    glBindBuffer(GL_ARRAY_BUFFER, smallWallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(smallWallVertices), smallWallVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)offsetof(Vertex3DColor, position));

    glEnableVertexAttribArray(VERTEX_ATTR_COLOR);
    glVertexAttribPointer(VERTEX_ATTR_COLOR, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)offsetof(Vertex3DColor, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /****************
     * PASSAGE WALLS
     ****************/

    Vertex3DColor passageWallVertices[] = {
        Vertex3DColor(glm::vec3(-1.f, -3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(1.f, -3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(1.f, 3.f, 0.f), glm::vec3(1.f, 0.25f, 0.2f)),
        Vertex3DColor(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f)),
        Vertex3DColor(glm::vec3(-1.f, -3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f)),
        Vertex3DColor(glm::vec3(1.f, 3.f, 0.f), glm::vec3(0.7f, 0.6f, 0.2f))};

    /* VBO & VAO */
    GLuint passageWallVBO, passageWallVAO;
    glGenVertexArrays(1, &passageWallVAO);
    glGenBuffers(1, &passageWallVBO);

    glBindVertexArray(passageWallVAO);

    glBindBuffer(GL_ARRAY_BUFFER, passageWallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(passageWallVertices), passageWallVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)offsetof(Vertex3DColor, position));

    glEnableVertexAttribArray(VERTEX_ATTR_COLOR);
    glVertexAttribPointer(VERTEX_ATTR_COLOR, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)offsetof(Vertex3DColor, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*****************
         * RENDERING CODE
         *****************/

        glm::mat4 ViewMatrix = camera.getViewMatrix();
        glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), (float)window_width / window_height, 0.1f, 100.f);
        glm::mat4 MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, -5));
        glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));

        // Sphere
        sphereProgram.use();
        glBindVertexArray(vao);

        MVMatrix = glm::rotate(MVMatrix, (float)glfwGetTime() * 0.5f, glm::vec3(0, 1, 0)); // Translation * Rotation
        glUniformMatrix4fv(sphereMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(sphereMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(sphereNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, sphere.getVertexCount());
        glBindVertexArray(0);

        // Floor
        squareProgram.use();
        glBindVertexArray(floorVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, -3, -17));
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(1, 0, 0));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 1 Back wall
        squareProgram.use();
        glBindVertexArray(wallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, 4)); // Position in front
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 1 Left wall
        squareProgram.use();
        glBindVertexArray(wallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-12, 0, -8)); // Position in front
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 1 Right wall
        squareProgram.use();
        glBindVertexArray(wallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(12, 0, -8)); // Position in front
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 1 Small left wall
        squareProgram.use();
        glBindVertexArray(smallWallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-7, 0, -16));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 1 Small right wall
        squareProgram.use();
        glBindVertexArray(smallWallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(7, 0, -16));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Passage walls
        squareProgram.use();
        glBindVertexArray(passageWallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-2, 0, -17));
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        squareProgram.use();
        glBindVertexArray(passageWallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(2, 0, -17));
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 2 back wall
        squareProgram.use();
        glBindVertexArray(wallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, -38)); // Position in front
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 2 Left wall
        squareProgram.use();
        glBindVertexArray(wallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-12, 0, -26)); // Position in front
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 2 Right wall
        squareProgram.use();
        glBindVertexArray(wallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(12, 0, -26)); // Position in front
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 2 Small left wall
        squareProgram.use();
        glBindVertexArray(smallWallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-7, 0, -18));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Room 2 Small right wall
        squareProgram.use();
        glBindVertexArray(smallWallVAO);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(7, 0, -18));
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glDeleteBuffers(1, &floorVBO);
    glDeleteVertexArrays(1, &floorVAO);

    glfwTerminate();

    return 0;
}