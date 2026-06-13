#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <assert.h>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void scroll_callback1(GLFWwindow *window, double xoffset, double yoffset, float &fov)
{
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

void mouse_callback1(GLFWwindow *window, double xpos, double ypos, bool &firstMouse, float &lastX, float &lastY, float &yaw, float &pitch, glm::vec3 &cameraFront, glm::vec3 &cameraUp)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    float sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(cameraFront,
                                                glm::vec3(0.0, 1.0, 0.0)));
    cameraUp = glm::normalize(glm::cross(right, cameraFront));
}

void key_callback1(GLFWwindow *window, int key, int scancode, int action, int mode, bool &rotateX, bool &rotateY, bool &rotateZ, bool &translateXPlus, bool &translateXMinus, bool &translateYPlus, bool &translateYMinus, bool &translateZPlus, bool &translateZMinus, bool &scalePlus, bool &scaleMinus, int &principalObject, std::vector<Object> &objects, bool lightEnabledArr[3], int &id)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X)
    {
        if (action == GLFW_PRESS)
            rotateX = true;
        else if (action == GLFW_RELEASE)
            rotateX = false;
    }

    if (key == GLFW_KEY_Y)
    {
        if (action == GLFW_PRESS)
            rotateY = true;
        else if (action == GLFW_RELEASE)
            rotateY = false;
    }

    if (key == GLFW_KEY_Z)
    {
        if (action == GLFW_PRESS)
            rotateZ = true;
        else if (action == GLFW_RELEASE)
            rotateZ = false;
    }

    if (key == GLFW_KEY_W)
    {
        if (action == GLFW_PRESS)
            translateYPlus = true;
        else if (action == GLFW_RELEASE)
            translateYPlus = false;
    }

    if (key == GLFW_KEY_S)
    {
        if (action == GLFW_PRESS)
            translateYMinus = true;
        else if (action == GLFW_RELEASE)
            translateYMinus = false;
    }

    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS)
            translateXPlus = true;
        else if (action == GLFW_RELEASE)
            translateXPlus = false;
    }

    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS)
            translateXMinus = true;
        else if (action == GLFW_RELEASE)
            translateXMinus = false;
    }

    if (key == GLFW_KEY_I)
    {
        if (action == GLFW_PRESS)
            translateZPlus = true;
        else if (action == GLFW_RELEASE)
            translateZPlus = false;
    }

    if (key == GLFW_KEY_J)
    {
        if (action == GLFW_PRESS)
            translateZMinus = true;
        else if (action == GLFW_RELEASE)
            translateZMinus = false;
    }

    if (key == GLFW_KEY_RIGHT_BRACKET)
    {
        if (action == GLFW_PRESS)
            scalePlus = true;
        else if (action == GLFW_RELEASE)
            scalePlus = false;
    }

    if (key == GLFW_KEY_LEFT_BRACKET)
    {
        if (action == GLFW_PRESS)
            scaleMinus = true;
        else if (action == GLFW_RELEASE)
            scaleMinus = false;
    }

    if (key == GLFW_KEY_RIGHT)
    {
        if (action == GLFW_PRESS)
            if (principalObject < objects.size() - 1)
            {
                principalObject += 1;
            }
            else
            {
                principalObject = 0;
            }
    }

    if (key == GLFW_KEY_LEFT)
    {
        if (action == GLFW_PRESS)
            if (principalObject > 0)
            {
                principalObject -= 1;
            }
            else
            {
                principalObject = objects.size() - 1;
            }
    }

    int getArrayLength = objects.size();
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        Object obj;
        obj.pathIndex = 0;
        obj.id = id;
        id += 1;
        objects.push_back(obj);
        principalObject = objects.size() - 1;
    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS && getArrayLength > 0)
    {
        objects.pop_back();
        principalObject = objects.size() - 1;
    }

    // Toggle lights: 1 = key, 2 = fill, 3 = back
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        lightEnabledArr[0] = !lightEnabledArr[0];
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        lightEnabledArr[1] = !lightEnabledArr[1];
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        lightEnabledArr[2] = !lightEnabledArr[2];
    }

    // CAMINHOO
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        BezierPoint point;
        point.x = objects[principalObject].x;
        point.y = objects[principalObject].y; 
        point.z = objects[principalObject].z;  
        point.scaleFactor = objects[principalObject].scaleFactor;
        point.rotateX = objects[principalObject].angleX;
        point.rotateY = objects[principalObject].angleY;
        point.rotateZ = objects[principalObject].angleZ;
        std::cout << "Adicionando ponto no caminho: (x:" << point.x << ", y:" << point.y << ", z:" << point.z << ", escala:" << point.scaleFactor << ", rotação X:" << point.rotateX << ", rotação Y:" << point.rotateY << ", rotação Z:" << point.rotateZ << "). Para o objeto: " << objects[principalObject].id << std::endl;
        objects[principalObject].path.push_back(point);
    }

    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        objects[principalObject].pathFollow = !objects[principalObject].pathFollow;
        objects[principalObject].pathFollow ? std::cout << "Seguindo caminho. Para o objeto: " << objects[principalObject].id << std::endl : std::cout << "Parou de seguir caminho. Para o objeto: " << objects[principalObject].id << std::endl;
    }

    
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        std::cout << "Resetando caminho" << std::endl;
         objects[principalObject].path.clear();
         objects[principalObject].pathIndex = 0;
         objects[principalObject].pathFollow = false;
    }
}

