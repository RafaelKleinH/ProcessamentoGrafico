#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <assert.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils.cpp"
#include "Object.cpp"
#include "callbackUtils.cpp"

bool loadMTL(const std::string &directory, const std::string &mtlFilename, const std::string &activeMaterial,
             glm::vec3 &Ka, glm::vec3 &Kd, glm::vec3 &Ks, float &q, std::string &diffuseMapPath);
void bezierPoint(float t,Object &obj, float deltaTime);

#include "objectUtils.cpp"

const GLuint WIDTH = 1920, HEIGHT = 1080;

const GLchar *vertexShaderSource = "#version 450\n"
                                   "layout (location = 0) in vec3 position;\n"
                                   "layout (location = 1) in vec3 vertexColor;\n"
                                   "layout (location = 2) in vec2 tex_coord;\n"
                                   "layout (location = 3) in vec3 normal;\n"

                                   "uniform mat4 model;\n"
                                   "uniform mat4 view;\n"
                                   "uniform mat4 projection;\n"
                                   "\n"
                                   "out vec3 Color;\n"
                                   "out vec2 TexCoord;\n"
                                   "out vec3 vNormal;\n"
                                   "out vec3 vFragPos;\n"
                                   "\n"
                                   "void main()\n"
                                   "{\n"
                                   "vFragPos = vec3(model * vec4(position, 1.0));\n"
                                   "vNormal = mat3(transpose(inverse(model))) * normal;\n"
                                   "Color = vertexColor;\n"
                                   "TexCoord = tex_coord;\n"
                                   "gl_Position = projection * view * model * vec4(position, 1.0);\n"
                                   "}\0";

const GLchar *fragmentShaderSource = "#version 450\n"
                                     "\n"
                                     "in vec3 Color;\n"
                                     "in vec2 TexCoord;\n"
                                     "in vec3 vNormal;\n"
                                     "in vec3 vFragPos;\n"
                                     "\n"

                                     "out vec4 color;\n"
                                     "\n"
                                     "uniform vec4 objectColor;\n"
                                     "uniform sampler2D tex_buffer;\n"
                                     "\n"
                                     "uniform vec3 Ka;\n"
                                     "uniform vec3 Kd;\n"
                                     "uniform vec3 Ks;\n"
                                     "uniform float q;\n"
                                     "\n"
                                     "const int NUM_LIGHTS = 3;\n"
                                     "uniform int numLights;\n"
                                     "uniform vec3 lightPos[NUM_LIGHTS];\n"
                                     "uniform vec3 lightColor[NUM_LIGHTS];\n"
                                     "uniform int lightOn[NUM_LIGHTS];\n"
                                     "uniform float attenConst;\n"
                                     "uniform float attenLinear;\n"
                                     "uniform float attenQuad;\n"
                                     "uniform vec3 viewPos;\n"
                                     "\n"
                                     "void main()\n"
                                     "{\n"
                                     "vec3 norm = normalize(vNormal);\n"
                                     "vec3 viewDir = normalize(viewPos - vFragPos);\n"
                                     "vec3 ambient = vec3(0.0);\n"
                                     "vec3 diffuse = vec3(0.0);\n"
                                     "vec3 specular = vec3(0.0);\n"
                                     "for (int i = 0; i < numLights; ++i) {\n"
                                     "if (lightOn[i] == 0) continue;\n"
                                     "vec3 L = normalize(lightPos[i] - vFragPos);\n"
                                     "float dist = length(lightPos[i] - vFragPos);\n"
                                     "float att = 1.0 / (attenConst + attenLinear * dist + attenQuad * dist * dist);\n"
                                     "float diff = max(dot(norm, L), 0.0);\n"
                                     "diffuse += Kd * diff * lightColor[i] * att;\n"
                                     "vec3 R = reflect(-L, norm);\n"
                                     "float s = pow(max(dot(viewDir, R), 0.0), q);\n"
                                     "specular += Ks * s * lightColor[i] * att;\n"
                                     "ambient += Ka * lightColor[i] * 0.1;\n"
                                     "}\n"
                                     "vec3 baseColor = texture(tex_buffer, TexCoord).rgb * objectColor.rgb;\n"
                                     "vec3 result = (ambient + diffuse) * baseColor + specular;\n"
                                     "color = vec4(result, objectColor.a);\n"
                                     "}\n\0";

int actualId = 0;
bool rotateX = false, rotateY = false, rotateZ = false, translateXPlus = false, translateXMinus = false, translateYPlus = false, translateYMinus = false, translateZPlus = false, translateZMinus = false, scalePlus = false, scaleMinus = false;
int principalObject = 0;
std::vector<Object> objects;
int imgWidth = 1024, imgHeight = 1024;
GLuint textId;
bool lightEnabledArr[3] = {true, true, true};

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;

float yaw = -90.0, pitch = 0.0;
float lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0;
float fov = 45.0f;

static void key_callback_glfw(GLFWwindow *window, int key, int scancode, int action, int mods);
static void mouse_callback_glfw(GLFWwindow *window, double xpos, double ypos);
static void scroll_callback_glfw(GLFWwindow *window, double xoffset, double yoffset);


int main()
{
    glfwInit();

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "AtivViv2", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback_glfw);
    glfwSetCursorPosCallback(window, mouse_callback_glfw);
    glfwSetScrollCallback(window, scroll_callback_glfw);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glm::vec3 Ka(0.1f, 0.1f, 0.1f);
    glm::vec3 Kd(1.0f, 1.0f, 1.0f);
    glm::vec3 Ks(0.5f, 0.5f, 0.5f);
    float q = 32.0f;
    std::string diffuseMapPath;

    int nVertices;    GLuint VAO = loadObj("../assets/Modelos3D/Suzanne.obj", nVertices, Ka, Kd, Ks, q, diffuseMapPath);

    std::string texturePath = diffuseMapPath.empty() ? "../assets/tex/pixelWall.png" : diffuseMapPath;
    textId = loadTexture(texturePath, imgWidth, imgHeight);
    if (textId == 0)
    {
        std::cout << "Failed to load texture " << texturePath << std::endl;
    }

    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderID = setupShader(const_cast<GLchar *>(vertexShaderSource), const_cast<GLchar *>(fragmentShaderSource));

    glUseProgram(shaderID);


    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint projectionLoc = glGetUniformLocation(shaderID, "projection");
    GLint colorLoc = glGetUniformLocation(shaderID, "objectColor");
    GLint texLoc = glGetUniformLocation(shaderID, "tex_buffer");

    GLint KaLoc = glGetUniformLocation(shaderID, "Ka");
    GLint KdLoc = glGetUniformLocation(shaderID, "Kd");
    GLint KsLoc = glGetUniformLocation(shaderID, "Ks");
    GLint qLoc = glGetUniformLocation(shaderID, "q");
    GLint numLightsLoc = glGetUniformLocation(shaderID, "numLights");
    GLint lightPosLoc = glGetUniformLocation(shaderID, "lightPos[0]");
    GLint lightColorLoc = glGetUniformLocation(shaderID, "lightColor[0]");
    GLint lightOnLoc = glGetUniformLocation(shaderID, "lightOn[0]");
    GLint attenConstLoc = glGetUniformLocation(shaderID, "attenConst");
    GLint attenLinearLoc = glGetUniformLocation(shaderID, "attenLinear");
    GLint attenQuadLoc = glGetUniformLocation(shaderID, "attenQuad");
    GLint viewPosLoc = glGetUniformLocation(shaderID, "viewPos");

    glUniform3fv(KaLoc, 1, glm::value_ptr(Ka));
    glUniform3fv(KdLoc, 1, glm::value_ptr(Kd));
    glUniform3fv(KsLoc, 1, glm::value_ptr(Ks));
    glUniform1f(qLoc, q);

    int numLights = 3;
    glUniform1i(numLightsLoc, numLights);
    glm::vec3 lightColors[3] = {glm::vec3(-4.0f, 3.0f, 5.0f), glm::vec3(4.0f, 1.0f, 2.0f), glm::vec3(0.0f, 5.0f, -5.0f)};
    if (lightColorLoc != -1)
        glUniform3fv(lightColorLoc, numLights, glm::value_ptr(lightColors[0]));

    if (attenConstLoc != -1)
        glUniform1f(attenConstLoc, 1.0f);
    if (attenLinearLoc != -1)
        glUniform1f(attenLinearLoc, 0.09f);
    if (attenQuadLoc != -1)
        glUniform1f(attenQuadLoc, 0.032f);

    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glUniform1i(texLoc, 0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    GLfloat clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textId);

    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f);
        if (projectionLoc != -1)
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


        float cameraSpeed = 3.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        if (viewLoc != -1)
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

        glLineWidth(10);
        glPointSize(20);

        glm::vec3 basePos(0.0f);
        float baseScale = 1.0f;
        if (!objects.empty() && principalObject >= 0 && principalObject < objects.size())
        {
            basePos = glm::vec3(objects[principalObject].x, objects[principalObject].y, objects[principalObject].z);
            baseScale = objects[principalObject].scaleFactor;
        }

        glm::vec3 lightPositions[3];

        lightPositions[0] = basePos + glm::vec3(-3.0f, 2.0f, 4.0f);
        lightPositions[1] = basePos + glm::vec3(3.0f, 1.0f, 3.0f);
        lightPositions[2] = basePos + glm::vec3(0.0f, 4.0f, -4.0f);

        if (lightPosLoc != -1)
            glUniform3fv(lightPosLoc, 3, glm::value_ptr(lightPositions[0]));
        int lightOnInts[3] = {lightEnabledArr[0] ? 1 : 0, lightEnabledArr[1] ? 1 : 0, lightEnabledArr[2] ? 1 : 0};
        if (lightOnLoc != -1)
            glUniform1iv(lightOnLoc, 3, lightOnInts);

        glBindVertexArray(VAO);
        for (int i = 0; i < objects.size(); i++)
        {

            if (i == principalObject)
            {
                if (translateXPlus)
                    objects[i].x += 0.01f;
                if (translateXMinus)
                    objects[i].x -= 0.01f;
                if (translateYPlus)
                    objects[i].y += 0.01f;
                if (translateYMinus)
                    objects[i].y -= 0.01f;
                if (translateZPlus)
                    objects[i].z += 0.01f;
                if (translateZMinus)
                    objects[i].z -= 0.01f;
                if (scalePlus)
                    objects[i].scaleFactor += 0.01f;
                if (scaleMinus)
                    objects[i].scaleFactor -= 0.01f;

                objects[i].x = verifyValue(objects[i].x);
                objects[i].y = verifyValue(objects[i].y);
                objects[i].z = verifyValue(objects[i].z);

                if (rotateX)
                    objects[i].angleX += 0.01f;
                if (rotateY)
                    objects[i].angleY += 0.01f;
                if (rotateZ)
                    objects[i].angleZ += 0.01f;

                objects[i].scaleFactor = glm::clamp(objects[i].scaleFactor, 0.1f, 3.0f);
            }

            if (objects[i].pathFollow && !objects[i].path.empty())
            {
                bezierPoint(1.0f, objects[i], deltaTime);
            }

            objects[i].model = glm::mat4(1);
            objects[i].model = glm::translate(objects[i].model, glm::vec3(objects[i].x, objects[i].y, objects[i].z));
            objects[i].model = glm::rotate(objects[i].model, objects[i].angleX, glm::vec3(1.0f, 0.0f, 0.0f));
            objects[i].model = glm::rotate(objects[i].model, objects[i].angleY, glm::vec3(0.0f, 1.0f, 0.0f));
            objects[i].model = glm::rotate(objects[i].model, objects[i].angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
            objects[i].model = glm::scale(objects[i].model, glm::vec3(objects[i].scaleFactor));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objects[i].model));

            glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

            glDrawElements(GL_TRIANGLES, nVertices, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}

static void key_callback_glfw(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    key_callback1(window, key, scancode, action, mods,
                 rotateX, rotateY, rotateZ,
                 translateXPlus, translateXMinus,
                 translateYPlus, translateYMinus,
                 translateZPlus, translateZMinus,
                 scalePlus, scaleMinus,
                 principalObject, objects, lightEnabledArr, actualId);
}

static void mouse_callback_glfw(GLFWwindow *window, double xpos, double ypos)
{
    mouse_callback1(window, xpos, ypos, firstMouse, lastX, lastY, yaw, pitch, cameraFront, cameraUp);
}

static void scroll_callback_glfw(GLFWwindow *window, double xoffset, double yoffset)
{
    scroll_callback1(window, xoffset, yoffset, fov);
}

void bezierPoint(float t,Object &obj, float deltaTime) {
        glm::vec4 current(obj.x, obj.y, obj.z, obj.scaleFactor);
        glm::vec4 target = glm::vec4(obj.path[obj.pathIndex].x, obj.path[obj.pathIndex].y, obj.path[obj.pathIndex].z, obj.path[obj.pathIndex].scaleFactor);
        glm::vec4 direction = target - current;
        float distance = glm::length(direction);
        
        bool distanceReach = true;
        bool angleReach = true;

        if (distance > 0.0001f) {
            glm::vec4 move = glm::normalize(direction) * t * deltaTime;

            if (glm::length(move) >= distance || distance < 0.01f) { 
                obj.x = target.x;
                obj.y = target.y;
                obj.z = target.z;
                obj.scaleFactor = target.w;
                std::cout << "Chegou ao ponto: " << obj.pathIndex << ". Para o objeto: " << obj.id << std::endl;
                distanceReach = true;
            } else {
                distanceReach = false;
                obj.x += move.x;
                obj.y += move.y;
                obj.z += move.z;
                obj.scaleFactor += move.w;
            }
        }

        glm::vec3 angleCurrent = glm::vec3(obj.angleX, obj.angleY, obj.angleZ);
        glm::vec3 angleTarget = glm::vec3(obj.path[obj.pathIndex].rotateX, obj.path[obj.pathIndex].rotateY, obj.path[obj.pathIndex].rotateZ);
        glm::vec3 angleDirection = angleTarget - angleCurrent;
        float angleDistance = glm::length(angleDirection);

        if (angleDistance > 0.0001f) {
            glm::vec3 angleMove = glm::normalize(angleDirection) * t * deltaTime;

            if (glm::length(angleMove) >= angleDistance || angleDistance < 0.01f) { 
                obj.angleX = angleTarget.x;
                obj.angleY = angleTarget.y;
                obj.angleZ = angleTarget.z;
                std::cout << "Chegou ao ponto: " << obj.pathIndex << ". Para o objeto: " << obj.id << std::endl;
                angleReach = true;
            } else {
                angleReach = false;
                obj.angleX += angleMove.x;
                obj.angleY += angleMove.y;
                obj.angleZ += angleMove.z;
            }
        }
        

        if (distanceReach && angleReach) {
            obj.pathIndex = (obj.pathIndex + 1) % obj.path.size();
        }
    }   