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
#include <src/stb_image.h>

using namespace glimac;

int window_width = 800;
int window_height = 800;

const GLuint VERTEX_ATTR_POSITION = 0;
const GLuint VERTEX_ATTR_NORMAL = 1;
const GLuint VERTEX_ATTR_COLOR = 2;

bool move = false;
float cameraHeight = 0.f;
FreeflyCamera camera;

bool line = false;

bool room1 = true;

struct Vertex3DColor
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;

    Vertex3DColor(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec4 &color)
        : position(position), normal(normal), color(color)
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

void initRecVBOandVAO(GLuint &vbo, GLuint &vao, const Vertex3DColor vertices[], GLsizeiptr size)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, position));

    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, normal));

    glEnableVertexAttribArray(VERTEX_ATTR_COLOR);
    glVertexAttribPointer(VERTEX_ATTR_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3DColor), (const GLvoid *)offsetof(Vertex3DColor, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawRec(GLuint vao, const glm::mat4 &MVMatrix, const glm::mat4 &ProjMatrix, GLint MVPMatrixLocation, GLint MVMatrixLocation, GLint NormalMatrixLocation)
{
    glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
    glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
    glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVMatrix));
    glUniformMatrix4fv(NormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void drawRec2(GLuint vao, const glm::mat4 &MVMatrix, const glm::mat4 &ProjMatrix, GLint MVPMatrixLocation)
{
    glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

float calculateDistance(const glm::vec3 &cameraPosition, const glm::vec3 &objectPosition)
{
    return glm::length(cameraPosition - objectPosition);
}

GLuint loadCubemap(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
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

    /* Load shaders */

    FilePath applicationPath(argv[0]);

    /* Skybox shaders */
    Program skyboxProgram = loadProgram(applicationPath.dirPath() + "../src/shaders/skybox.vs.glsl",
                                        applicationPath.dirPath() + "../src/shaders/skybox.fs.glsl");

    /* Room 1 Shaders */
    Program room1Program = loadProgram(applicationPath.dirPath() + "../src/shaders/room1.vs.glsl",
                                       applicationPath.dirPath() + "../src/shaders/room1.fs.glsl");

    GLint room1MVPMatrixLocation = glGetUniformLocation(room1Program.getGLId(), "uMVPMatrix");
    GLint room1MVMatrixLocation = glGetUniformLocation(room1Program.getGLId(), "uMVMatrix");
    GLint room1NormalMatrixLocation = glGetUniformLocation(room1Program.getGLId(), "uNormalMatrix");

    /* Room 2 Shaders */
    Program room2Program = loadProgram(applicationPath.dirPath() + "../src/shaders/room2.vs.glsl",
                                       applicationPath.dirPath() + "../src/shaders/room2.fs.glsl");
    room2Program.use();

    GLint room2MVPMatrixLocation = glGetUniformLocation(room2Program.getGLId(), "uMVPMatrix");

    glEnable(GL_DEPTH_TEST);

    /* Hook input callbacks */
    glfwSetKeyCallback(window, &key_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetCursorPosCallback(window, &cursor_position_callback);
    glfwSetWindowSizeCallback(window, &size_callback);

    /***********************
     * INITIALIZATION CODE
     ***********************/

    /*********
     * FLOOR
     *********/

    Vertex3DColor floorVertices[] = {
        Vertex3DColor(glm::vec3(-12.f, -21.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, -21.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 21.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, 21.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, -21.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 21.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.4f, 0.25f, 0.2f, 1.f))};

    /* VBO & VAO */
    GLuint floorVBO, floorVAO;
    initRecVBOandVAO(floorVBO, floorVAO, floorVertices, sizeof(floorVertices));

    /********
     * WALLS
     ********/

    // Mur arrière
    Vertex3DColor backWallVertices[] = {
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f))};

    // Mur gauche
    Vertex3DColor leftWallVertices[] = {
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f))};

    // Mur droit
    Vertex3DColor rightWallVertices[] = {
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-12.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(12.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f))};

    // Initialisation des VBO et VAO pour chaque mur
    GLuint backWallVBO, backWallVAO;
    initRecVBOandVAO(backWallVBO, backWallVAO, backWallVertices, sizeof(backWallVertices));

    GLuint leftWallVBO, leftWallVAO;
    initRecVBOandVAO(leftWallVBO, leftWallVAO, leftWallVertices, sizeof(leftWallVertices));

    GLuint rightWallVBO, rightWallVAO;
    initRecVBOandVAO(rightWallVBO, rightWallVAO, rightWallVertices, sizeof(rightWallVertices));

    /**************
     * SMALL WALLS
     **************/
    Vertex3DColor smallWallVertices[] = {
        Vertex3DColor(glm::vec3(-5.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(5.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(5.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-5.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-5.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(5.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f))};

    /* VBO & VAO */
    GLuint smallWallVBO, smallWallVAO;
    initRecVBOandVAO(smallWallVBO, smallWallVAO, smallWallVertices, sizeof(smallWallVertices));

    /****************
     * PASSAGE WALLS
     ****************/

    Vertex3DColor leftPassageWallVertices[] = {
        Vertex3DColor(glm::vec3(-1.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, -3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f))};

    Vertex3DColor rightPassageWallVertices[] = {
        Vertex3DColor(glm::vec3(-1.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, -3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 3.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(0.5f, 0.6f, 0.7f, 1.f))};

    /* VBO & VAO */
    GLuint leftPassageWallVBO, leftPassageWallVAO;
    initRecVBOandVAO(leftPassageWallVBO, leftPassageWallVAO, leftPassageWallVertices, sizeof(leftPassageWallVertices));

    GLuint rightPassageWallVBO, rightPassageWallVAO;
    initRecVBOandVAO(rightPassageWallVBO, rightPassageWallVAO, rightPassageWallVertices, sizeof(rightPassageWallVertices));

    /*********
     * WINDOW
     *********/

    Vertex3DColor windowVertices[] = {
        Vertex3DColor(glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(-0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 0.15f)),
        Vertex3DColor(glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 0.15f))};

    /* VBO & VAO */
    GLuint windowVBO, windowVAO;
    initRecVBOandVAO(windowVBO, windowVAO, windowVertices, sizeof(windowVertices));

    /***********
     * PEDESTAL
     ***********/

    Vertex3DColor pedestalVertices[] = {
        Vertex3DColor(glm::vec3(-1.f, -1.25f, -1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, -1.25f, -1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 1.25f, -1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, 1.25f, -1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, -1.25f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, -1.25f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(1.f, 1.25f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
        Vertex3DColor(glm::vec3(-1.f, 1.25f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f)),
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
    {
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
    }

    /**********
     * SKYBOX
     **********/

    float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    // Load skybox textures
    std::vector<std::string> faces{
        applicationPath.dirPath() + "/assets/skybox/right.jpg",
        applicationPath.dirPath() + "/assets/skybox/left.jpg",
        applicationPath.dirPath() + "/assets/skybox/top.jpg",
        applicationPath.dirPath() + "/assets/skybox/bottom.jpg",
        applicationPath.dirPath() + "/assets/skybox/front.jpg",
        applicationPath.dirPath() + "/assets/skybox/back.jpg"};
    GLuint cubemapTexture = loadCubemap(faces);

    // Skybox VAO and VBO
    GLuint skyboxVAO, skyboxVBO;
    {
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glBindVertexArray(0);
    }

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

        /* Skybox */
        {
            glDepthFunc(GL_LEQUAL);
            skyboxProgram.use();
            glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix()));
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);
            glUniformMatrix4fv(glGetUniformLocation(skyboxProgram.getGLId(), "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(skyboxProgram.getGLId(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(skyboxVAO);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS);
        }

        /*****************
         * SCENE GEOMETRY
         *****************/

        if (camera.getPosition().z > -17)
        {
            room1Program.use();
            room1 = true;

            // Set light positions in world space
            glm::vec3 lightPos1_world = glm::vec3(8.0f, 0.f, 0.f);
            glm::vec3 lightPos2_world = glm::vec3(0.f, 0.f, -13.f);

            // Transform light positions to view space
            glm::vec3 lightPos1_vs = glm::vec3(ViewMatrix * glm::vec4(lightPos1_world, 1.0f));
            glm::vec3 lightPos2_vs = glm::vec3(ViewMatrix * glm::vec4(lightPos2_world, 1.0f));

            // Set light uniforms
            GLint uLightPos1_vs = glGetUniformLocation(room1Program.getGLId(), "uLightPos1_vs");
            GLint uLightIntensity1 = glGetUniformLocation(room1Program.getGLId(), "uLightIntensity1");
            GLint uLightPos2_vs = glGetUniformLocation(room1Program.getGLId(), "uLightPos2_vs");
            GLint uLightIntensity2 = glGetUniformLocation(room1Program.getGLId(), "uLightIntensity2");

            glm::vec3 lightIntensity1 = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 lightIntensity2 = glm::vec3(0.5f, 0.5f, 0.5f);

            glUniform3fv(uLightPos1_vs, 1, glm::value_ptr(lightPos1_vs));
            glUniform3fv(uLightIntensity1, 1, glm::value_ptr(lightIntensity1));
            glUniform3fv(uLightPos2_vs, 1, glm::value_ptr(lightPos2_vs));
            glUniform3fv(uLightIntensity2, 1, glm::value_ptr(lightIntensity2));

            // Set material uniforms
            GLint uKd = glGetUniformLocation(room1Program.getGLId(), "uKd");
            GLint uKs = glGetUniformLocation(room1Program.getGLId(), "uKs");
            GLint uShininess = glGetUniformLocation(room1Program.getGLId(), "uShininess");

            glm::vec3 Kd = glm::vec3(0.8f, 0.8f, 0.8f);
            glm::vec3 Ks = glm::vec3(0.5f, 0.5f, 0.5f);
            float shininess = 32.0f;

            glUniform3fv(uKd, 1, glm::value_ptr(Kd));
            glUniform3fv(uKs, 1, glm::value_ptr(Ks));
            glUniform1f(uShininess, shininess);
        }
        else
        {
            room2Program.use();
            room1 = false;
        }

        {
            /* Floor */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, -3, -17));
            MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(1, 0, 0));
            (room1)
                ? drawRec(floorVAO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(floorVAO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 1 Back wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, 4)); // Position in front
            (room1)
                ? drawRec(backWallVBO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(backWallVBO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 1 Left wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(-12, 0, -8)); // Position in front
            MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
            (room1)
                ? drawRec(leftWallVBO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(leftWallVBO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 1 Right wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(12, 0, -8)); // Position in front
            MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
            (room1)
                ? drawRec(rightWallVBO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(rightWallVBO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 1 Small left wall */
            glBindVertexArray(smallWallVAO);
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(-7, 0, -16));
            (room1)
                ? drawRec(smallWallVAO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(smallWallVAO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 1 Small right wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(7, 0, -16));
            (room1)
                ? drawRec(smallWallVAO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(smallWallVAO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Passage walls */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(-2, 0, -17));
            MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
            (room1)
                ? drawRec(leftPassageWallVAO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(leftPassageWallVAO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            MVMatrix = glm::translate(ViewMatrix, glm::vec3(2, 0, -17));
            MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
            (room1)
                ? drawRec(rightPassageWallVAO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(rightPassageWallVAO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 2 back wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, -38)); // Position in front
            (room1)
                ? drawRec(backWallVBO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(backWallVBO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 2 Left wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(-12, 0, -26)); // Position in front
            MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
            (room1)
                ? drawRec(leftWallVBO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(leftWallVBO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 2 Right wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(12, 0, -26)); // Position in front
            MVMatrix = glm::rotate(MVMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
            (room1)
                ? drawRec(rightWallVAO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(rightWallVAO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 2 Small left wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(-7, 0, -18));
            (room1)
                ? drawRec(smallWallVAO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(smallWallVAO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);

            /* Room 2 Small right wall */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(7, 0, -18));
            (room1)
                ? drawRec(smallWallVAO, MVMatrix, ProjMatrix, room1MVPMatrixLocation, room1MVMatrixLocation, room1NormalMatrixLocation)
                : drawRec2(smallWallVAO, MVMatrix, ProjMatrix, room2MVPMatrixLocation);
        }

        /*****************
         * ROOM 2 OBJECTS
         *****************/

        {
            /* Pedestal */
            MVMatrix = glm::translate(ViewMatrix, glm::vec3(0, -1.75f, -24.75));
            glBindVertexArray(pedestalVAO);
            glUniformMatrix4fv(room2MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            /* Windows */
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
                drawRec2(obj.vao, obj.modelMatrix, ProjMatrix, room2MVPMatrixLocation);
            }
            glDisable(GL_BLEND);
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }
    // glDeleteBuffers(1, &vbo);
    // glDeleteVertexArrays(1, &vao);

    glDeleteBuffers(1, &floorVBO);
    glDeleteVertexArrays(1, &floorVAO);

    glDeleteBuffers(1, &smallWallVBO);
    glDeleteVertexArrays(1, &smallWallVAO);

    glfwTerminate();

    return 0;
}