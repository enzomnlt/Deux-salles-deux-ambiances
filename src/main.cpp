#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glimac/FilePath.hpp>
#include <glimac/Program.hpp>
#include <glimac/FreeflyCamera.hpp>
#include <glimac/Sphere.hpp>
#include <glimac/Cone.hpp>
#include <cstddef>
#include <algorithm>
#include <vector>

using namespace glimac;

int window_width = 800;
int window_height = 800;

const GLuint VERTEX_ATTR_POSITION = 0;
const GLuint VERTEX_ATTR_NORMAL = 1;
const GLuint VERTEX_ATTR_COLOR = 2;

bool move = false;
bool line = false;
FreeflyCamera camera;
float cameraHeight = 0.f;

struct Vertex3DColor
{
    glm::vec3 position;
    glm::vec4 color;

    Vertex3DColor(const glm::vec3 &position, const glm::vec4 &color)
        : position(position), color(color)
    {
    }
};

struct TransparentObject
{
    GLuint vao;
    glm::vec3 position;
    glm::mat4 modelMatrix;
};

static void key_callback(GLFWwindow * /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
    // Movement
    if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_W)
    {
        camera.moveFront(0.2);
    }
    else if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_S)
    {
        camera.moveFront(-0.2);
    }
    else if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_A)
    {
        camera.moveLeft(0.2);
    }
    else if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_D)
    {
        camera.moveLeft(-0.2);
    }
    camera.setCameraPositionY(cameraHeight);

    // Line mode
    if (action == GLFW_PRESS && key == GLFW_KEY_F)
    {
        (line) ? glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        line = !line;
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

void createWallVBOandVAO(GLuint &vbo, GLuint &vao, const Vertex3DColor vertices[], GLsizeiptr size)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, position));

    glEnableVertexAttribArray(VERTEX_ATTR_COLOR);
    glVertexAttribPointer(VERTEX_ATTR_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawWall(GLuint vao, const glm::mat4 &MVMatrix, const glm::mat4 &ProjMatrix, GLint MVPMatrixLocation, GLint MVMatrixLocation, GLint NormalMatrixLocation)
{
    glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
    glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
    glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
    glUniformMatrix4fv(NormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

float calculateDistance(const glm::vec3 &cameraPosition, const glm::vec3 &objectPosition)
{
    return glm::length(cameraPosition - objectPosition);
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

    /**********
     * Cone
     **********/

    Cone cone(2, 1, 32, 16);

    /* VBO */
    GLuint conevbo;
    glGenBuffers(1, &conevbo);
    glBindBuffer(GL_ARRAY_BUFFER, conevbo);
    glBufferData(GL_ARRAY_BUFFER, cone.getVertexCount() * sizeof(ShapeVertex), cone.getDataPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* VAO */
    GLuint conevao;
    glGenVertexArrays(1, &conevao);

    glBindVertexArray(conevao);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);

    glBindBuffer(GL_ARRAY_BUFFER, conevbo);

    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), (const GLvoid *)offsetof(ShapeVertex, position));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), (const GLvoid *)offsetof(ShapeVertex, normal));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    /*********
     * FLOOR
     *********/

    Vertex3DColor floorVertices[] = {
        Vertex3DColor(glm::vec3(-12.f, -21.f, 0.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, -21.f, 0.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 21.f, 0.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, 21.f, 0.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, -21.f, 0.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 21.f, 0.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f))};

    /* VBO & VAO */
    GLuint floorVBO, floorVAO;
    createWallVBOandVAO(floorVBO, floorVAO, floorVertices, sizeof(floorVertices));

    /********
     * WALLS
     ********/

    Vertex3DColor wallVertices[] = {
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, -3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, 3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f))};

    /* VBO & VAO */
    GLuint wallVBO, wallVAO;
    createWallVBOandVAO(wallVBO, wallVAO, wallVertices, sizeof(wallVertices));

    /**************
     * SMALL WALLS
     **************/
    Vertex3DColor smallWallVertices[] = {
        Vertex3DColor(glm::vec3(-5.f, -3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(5.f, -3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(5.f, 3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-5.f, 3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-5.f, -3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(5.f, 3.f, 0.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f))};

    /* VBO & VAO */
    GLuint smallWallVBO, smallWallVAO;
    createWallVBOandVAO(smallWallVBO, smallWallVAO, smallWallVertices, sizeof(smallWallVertices));

    /****************
     * PASSAGE WALLS
     ****************/

    Vertex3DColor passageWallVertices[] = {
        Vertex3DColor(glm::vec3(-1.f, -3.f, 0.f), glm::vec4(0.5f, 1.f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, -3.f, 0.f), glm::vec4(0.5f, 1.f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 3.f, 0.f), glm::vec4(0.5f, 1.f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, 3.f, 0.f), glm::vec4(0.5f, 1.f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, -3.f, 0.f), glm::vec4(0.5f, 1.f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 3.f, 0.f), glm::vec4(0.5f, 1.f, 0.7f, 1.f))};

    /* VBO & VAO */
    GLuint passageWallVBO, passageWallVAO;
    createWallVBOandVAO(passageWallVBO, passageWallVAO, passageWallVertices, sizeof(passageWallVertices));

    /*********
     * WINDOW
     *********/

    Vertex3DColor windowVertices[] = {
        Vertex3DColor(glm::vec3(-0.5f, -0.5f, 0.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(0.5f, -0.5f, 0.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(0.5f, 0.5f, 0.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(-0.5f, 0.5f, 0.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(-0.5f, -0.5f, 0.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(0.5f, 0.5f, 0.f), glm::vec4(1.f, 1.f, 1.f, 0.15f))};

    /* VBO & VAO */
    GLuint windowVBO, windowVAO;
    createWallVBOandVAO(windowVBO, windowVAO, windowVertices, sizeof(windowVertices));

    /***********
     * PEDESTAL
     ***********/

    Vertex3DColor pedestalVertices[] = {
        Vertex3DColor(glm::vec3(-1.f, -1.25f, -1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, -1.25f, -1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 1.25f, -1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, 1.25f, -1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, -1.25f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, -1.25f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 1.25f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, 1.25f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
    };

    GLuint pedestalIndices[] = {
        0, 1, 2, 2, 3, 0, // Front face
        4, 5, 6, 6, 7, 4, // Back face
        0, 1, 5, 5, 4, 0, // Bottom face
        2, 3, 7, 7, 6, 2, // Top face
        0, 3, 7, 7, 4, 0, // Left face
        1, 2, 6, 6, 5, 1  // Right face
    };

    GLuint pedestalVBO, pedestalVAO, pedestalEBO;
    glGenVertexArrays(1, &pedestalVAO);
    glGenBuffers(1, &pedestalVBO);
    glGenBuffers(1, &pedestalEBO);

    glBindVertexArray(pedestalVAO);

    glBindBuffer(GL_ARRAY_BUFFER, pedestalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pedestalVertices), pedestalVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pedestalEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pedestalIndices), pedestalIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, position));

    glEnableVertexAttribArray(VERTEX_ATTR_COLOR);
    glVertexAttribPointer(VERTEX_ATTR_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, color));

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
        glm::mat4 MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, 0));
        glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));

        // Sphere
        sphereProgram.use();
        glBindVertexArray(vao);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, -5));
        MVMatrix = glm::rotate(MVMatrix, (float)glfwGetTime() * 0.5f, glm::vec3(0, 1, 0)); // Translation * Rotation
        glUniformMatrix4fv(sphereMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(sphereMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(sphereNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, sphere.getVertexCount());
        glBindVertexArray(0);

        // Cone
        sphereProgram.use();
        glBindVertexArray(conevao);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, -2, -7));
        MVMatrix = glm::rotate(MVMatrix, (float)glfwGetTime() * 0.5f, glm::vec3(0, 1, 0)); // Translation * Rotation
        glUniformMatrix4fv(sphereMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(sphereMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(sphereNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, cone.getVertexCount());
        glBindVertexArray(0);

        // Cone
        sphereProgram.use();
        glBindVertexArray(conevao);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 1, -7));
        MVMatrix = glm::rotate(MVMatrix, glm::radians(180.f), glm::vec3(1, 0, 0));
        MVMatrix = glm::rotate(MVMatrix, (float)glfwGetTime() * -0.5f, glm::vec3(0, 1, 0)); // Translation * Rotation
        glUniformMatrix4fv(sphereMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(sphereMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(sphereNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, cone.getVertexCount());
        glBindVertexArray(0);

        /*****************
         * SCENE GEOMETRY
         *****************/

        squareProgram.use();

        // Floor
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, -3, -17));
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(1, 0, 0));
        drawWall(floorVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 1 Back wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, 4)); // Position in front
        drawWall(wallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 1 Left wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-12, 0, -8)); // Position in front
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        drawWall(wallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 1 Right wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(12, 0, -8)); // Position in front
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        drawWall(wallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 1 Small left wall
        glBindVertexArray(smallWallVAO);
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-7, 0, -16));
        drawWall(smallWallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 1 Small right wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(7, 0, -16));
        drawWall(smallWallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Passage walls
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-2, 0, -17));
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        drawWall(passageWallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        MVMatrix = glm::translate(ViewMatrix, glm::vec3(2, 0, -17));
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        drawWall(passageWallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 2 back wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, -38)); // Position in front
        drawWall(wallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 2 Left wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-12, 0, -26)); // Position in front
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        drawWall(wallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 2 Right wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(12, 0, -26)); // Position in front
        MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        drawWall(wallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 2 Small left wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(-7, 0, -18));
        drawWall(smallWallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Room 2 Small right wall
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(7, 0, -18));
        drawWall(smallWallVAO, MVMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);

        // Pedestal
        MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, -1.75f, -24.75));
        glBindVertexArray(pedestalVAO);
        glUniformMatrix4fv(squareMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
        glUniformMatrix4fv(squareMVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(squareNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Windows
        std::vector<TransparentObject> transparentObjects = {
            {windowVAO, glm::vec3(0, 0, -24), glm::translate(ViewMatrix, glm::vec3(0, 0, -24))},
            {windowVAO, glm::vec3(0, 0, -25.5), glm::translate(ViewMatrix, glm::vec3(0, 0, -25.5))},
            {windowVAO, glm::vec3(-0.75f, 0, -24.75f), glm::rotate(glm::translate(ViewMatrix, glm::vec3(-0.75f, 0, -24.75f)), glm::radians(90.f), glm::vec3(0, 1, 0))},
            {windowVAO, glm::vec3(0.75f, 0, -24.75f), glm::rotate(glm::translate(ViewMatrix, glm::vec3(0.75f, 0, -24.75f)), glm::radians(90.f), glm::vec3(0, 1, 0))}};

        glm::vec3 cameraPosition = camera.getPosition();

        std::sort(transparentObjects.begin(), transparentObjects.end(), [&cameraPosition](const TransparentObject &a, const TransparentObject &b)
                  { return calculateDistance(cameraPosition, a.position) > calculateDistance(cameraPosition, b.position); });

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (const auto &obj : transparentObjects)
        {
            drawWall(obj.vao, obj.modelMatrix, ProjMatrix, squareMVPMatrixLocation, squareMVMatrixLocation, squareNormalMatrixLocation);
        }
        glDisable(GL_BLEND);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glDeleteBuffers(1, &floorVBO);
    glDeleteVertexArrays(1, &floorVAO);

    glDeleteBuffers(1, &wallVBO);
    glDeleteVertexArrays(1, &wallVAO);

    glDeleteBuffers(1, &smallWallVBO);
    glDeleteVertexArrays(1, &smallWallVAO);

    glDeleteBuffers(1, &passageWallVBO);
    glDeleteVertexArrays(1, &passageWallVAO);

    glfwTerminate();

    return 0;
}