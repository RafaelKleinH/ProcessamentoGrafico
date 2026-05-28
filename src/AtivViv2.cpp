/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

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


struct Object
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
    float scaleFactor = 1.0f;
    glm::mat4 model;
};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
float verifyValue(float value);
int setupShader();
bool loadMTL(const std::string &directory, const std::string &mtlFilename, const std::string &activeMaterial, glm::vec3 &Ka, glm::vec3 &Kd, glm::vec3 &Ks, float &Ns, std::string &diffuseMapPath);
int loadObj(const char *path, int &numVertices, glm::vec3 &Ka, glm::vec3 &Kd, glm::vec3 &Ks, float &Ns, std::string &diffuseMapPath);

GLuint loadTexture(string path, int &width, int &height);

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

bool rotateX = false, rotateY = false, rotateZ = false, translateXPlus = false, translateXMinus = false, translateYPlus = false, translateYMinus = false, translateZPlus = false, translateZMinus = false, scalePlus = false, scaleMinus = false;
int principalObject = 0;
std::vector<Object> objects;
int imgWidth = 1024, imgHeight = 1024;
GLuint textId;
bool lightEnabledArr[3] = { true, true, true };

int main()
{
    glfwInit();

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Rossana!", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    // GLAD: carrega todos os ponteiros d funções da OpenGL
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

    int nVertices;
    GLuint VAO = loadObj("../assets/Modelos3D/Suzanne.obj", nVertices, Ka, Kd, Ks, q, diffuseMapPath);

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

    GLuint shaderID = setupShader();

    glUseProgram(shaderID);
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint projectionLoc = glGetUniformLocation(shaderID, "projection");
    GLint colorLoc = glGetUniformLocation(shaderID, "objectColor");
    GLint texLoc = glGetUniformLocation(shaderID, "tex_buffer");
  
    GLint KaLoc = glGetUniformLocation(shaderID, "Ka");
    GLint KdLoc = glGetUniformLocation(shaderID, "Kd");
    GLint KsLoc = glGetUniformLocation(shaderID, "Ks");
    GLint qLoc = glGetUniformLocation(shaderID, "q");
    // Uniforms de iluminação (arrays para múltiplas luzes)
    GLint numLightsLoc = glGetUniformLocation(shaderID, "numLights");
    GLint lightPosLoc = glGetUniformLocation(shaderID, "lightPos[0]");
    GLint lightColorLoc = glGetUniformLocation(shaderID, "lightColor[0]");
    GLint lightOnLoc = glGetUniformLocation(shaderID, "lightOn[0]");
    GLint attenConstLoc = glGetUniformLocation(shaderID, "attenConst");
    GLint attenLinearLoc = glGetUniformLocation(shaderID, "attenLinear");
    GLint attenQuadLoc = glGetUniformLocation(shaderID, "attenQuad");
    GLint viewPosLoc = glGetUniformLocation(shaderID, "viewPos");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glUniform3fv(KaLoc, 1, glm::value_ptr(Ka));      
    glUniform3fv(KdLoc, 1, glm::value_ptr(Kd));  
    glUniform3fv(KsLoc, 1, glm::value_ptr(Ks));   
    glUniform1f(qLoc, q);
    
    int numLights = 3;
    glUniform1i(numLightsLoc, numLights);
    glm::vec3 lightColors[3] = { glm::vec3(-4.0f, 3.0f, 5.0f), glm::vec3(4.0f, 1.0f, 2.0f), glm::vec3(0.0f, 5.0f, -5.0f) };
    if (lightColorLoc != -1) glUniform3fv(lightColorLoc, numLights, glm::value_ptr(lightColors[0]));

    if (attenConstLoc != -1) glUniform1f(attenConstLoc, 1.0f);
    if (attenLinearLoc != -1) glUniform1f(attenLinearLoc, 0.09f);
    if (attenQuadLoc != -1) glUniform1f(attenQuadLoc, 0.032f);

    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniform3f(viewPosLoc, camPos.x, camPos.y, camPos.z);
    glUniform1i(texLoc, 0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    GLfloat clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textId);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);
        
            glm::vec3 basePos(0.0f);
            float baseScale = 1.0f;
            if (!objects.empty() && principalObject >= 0 && principalObject < objects.size()) {
                basePos = glm::vec3(objects[principalObject].x, objects[principalObject].y, objects[principalObject].z);
                baseScale = objects[principalObject].scaleFactor;
            }

            glm::vec3 lightPositions[3];
            
            // keylight
            lightPositions[0] = basePos + glm::vec3(-3.0f, 2.0f, 4.0f);
            // filllight
            lightPositions[1] = basePos + glm::vec3(3.0f, 1.0f, 3.0f);
            // backlight
            lightPositions[2] = basePos + glm::vec3(0.0f, 4.0f, -4.0f);

            if (lightPosLoc != -1) glUniform3fv(lightPosLoc, 3, glm::value_ptr(lightPositions[0]));
            int lightOnInts[3] = { lightEnabledArr[0] ? 1 : 0, lightEnabledArr[1] ? 1 : 0, lightEnabledArr[2] ? 1 : 0 };
            if (lightOnLoc != -1) glUniform1iv(lightOnLoc, 3, lightOnInts);

            glBindVertexArray(VAO);
        for(int i=0; i < objects.size(); i++){
        
            if (i == principalObject) {
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

float verifyValue(float value)
{
    if (value > 1.5f)
    {
        return -1.5f;
    }
    else if (value < -1.5f)
    {
        return 1.5f;
    }
    return value;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
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
            if (principalObject < objects.size() - 1) {
                principalObject += 1;
            } else {
                principalObject = 0;
            }

    }

    if (key == GLFW_KEY_LEFT)
    {
        if (action == GLFW_PRESS)
            if (principalObject > 0) {
                principalObject -= 1;
            } else {
                principalObject = objects.size() - 1;
            }
    }

    int getArrayLength = objects.size();
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        Object obj;
        objects.push_back(obj);
        principalObject = objects.size() - 1;
    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS && getArrayLength > 0)
    {
        objects.pop_back();
        principalObject = objects.size() - 1;   
    }

    // Toggle lights: 1 = key, 2 = fill, 3 = back
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        lightEnabledArr[0] = !lightEnabledArr[0];
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        lightEnabledArr[1] = !lightEnabledArr[1];
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        lightEnabledArr[2] = !lightEnabledArr[2];
    }
}

int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

bool loadMTL(const std::string &directory, const std::string &mtlFilename, const std::string &activeMaterial, glm::vec3 &Ka, glm::vec3 &Kd, glm::vec3 &Ks, float &q, std::string &diffuseMapPath)
{
    std::string mtlPath = directory + "/" + mtlFilename;
    FILE *file = fopen(mtlPath.c_str(), "r");
    if (file == NULL)
    {
        printf("MTL file not found: %s\n", mtlPath.c_str());
        return false;  // Arquivo não encontrado
    }

    bool foundMaterial = false;
    std::string currentMaterial;
    while (true)
    {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader); 
        if (res == EOF)
            break;
        if (strcmp(lineHeader, "newmtl") == 0) {
            fscanf(file, "%127s\n", lineHeader);
            currentMaterial = lineHeader;

            if (currentMaterial == activeMaterial)
                foundMaterial = true; 
        } else if (foundMaterial) {
            if (strcmp(lineHeader, "Ka") == 0) {
                fscanf(file, "%f %f %f\n", &Ka.x, &Ka.y, &Ka.z);
            } else if (strcmp(lineHeader, "Kd") == 0) {
                fscanf(file, "%f %f %f\n", &Kd.x, &Kd.y, &Kd.z);
            } else if (strcmp(lineHeader, "Ks") == 0) {
                fscanf(file, "%f %f %f\n", &Ks.x, &Ks.y, &Ks.z);
            } else if (strcmp(lineHeader, "Ns") == 0) {
                fscanf(file, "%f\n", &q);
            } else if (strcmp(lineHeader, "map_Kd") == 0) {
                char mapPath[256];
                fscanf(file, "%255s\n", mapPath);
                diffuseMapPath = directory + "/" + mapPath; 
            } else if (strcmp(lineHeader, "newmtl") == 0) {
                break;
            } else {
                char buffer[256];
                fgets(buffer, sizeof(buffer), file);
            }
        } else {
            char buffer[256];
            fgets(buffer, sizeof(buffer), file);
        }
    }

    fclose(file);
    return foundMaterial;
}

int loadObj(const char *path, int &numVertices, glm::vec3 &Ka, glm::vec3 &Kd, glm::vec3 &Ks, float &q, std::string &diffuseMapPath)
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices; 
    std::vector<glm::vec3> temp_vertices; 
    std::vector<glm::vec2> temp_uvs;        
    std::vector<glm::vec3> temp_normals;   
    std::vector<unsigned int> indices;  

    std::vector<float> out_data;

    std::string pathStr(path);
    std::string directory = pathStr.substr(0, pathStr.find_last_of("/\\"));
    std::string filename = pathStr.substr(pathStr.find_last_of("/\\") + 1);

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return -1;  // Erro: arquivo não encontrado
    }

    std::string mtlFilename;   
    std::string activeMaterial;

    while (1) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) {
            break;  // Fim do arquivo
        }

       
        if (strcmp(lineHeader, "mtllib") == 0) {
            fscanf(file, "%127s\n", lineHeader);
            mtlFilename = lineHeader;
        } else if (strcmp(lineHeader, "usemtl") == 0) {
            fscanf(file, "%127s\n", lineHeader);
            activeMaterial = lineHeader;
        } else if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        } else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = 1.0f - uv.y;
            temp_uvs.push_back(uv);
        } else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        } else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9)
            {
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                fclose(file);
                return -1;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        } else {
            char buffer[256];
            fgets(buffer, sizeof(buffer), file);
        }
    }

    fclose(file);

    if (!mtlFilename.empty())
    {
        loadMTL(directory, mtlFilename, activeMaterial, Ka, Kd, Ks, q, diffuseMapPath);
    }

    if (vertexIndices.empty())
    {
        printf("No face data found in OBJ file.\n");
        return -1;
    }

    for (unsigned int i = 0; i < vertexIndices.size(); i++)
    {
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        if (vertexIndex == 0 || vertexIndex > temp_vertices.size())
        {
            printf("Vertex index out of range: %u (vertices: %zu)\n", vertexIndex, temp_vertices.size());
            return -1;
        }

        glm::vec3 pos = temp_vertices[vertexIndex - 1];  
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec2 uv = (uvIndex > 0 && uvIndex <= temp_uvs.size()) ? temp_uvs[uvIndex - 1] : glm::vec2(0.0f, 0.0f);
        glm::vec3 normal = (normalIndex > 0 && normalIndex <= temp_normals.size()) ? temp_normals[normalIndex - 1] : glm::vec3(0.0f, 1.0f, 0.0f);

        out_data.push_back(pos.x);        // 0-2: Posição
        out_data.push_back(pos.y);
        out_data.push_back(pos.z);
        out_data.push_back(color.r);      // 3-5: Cor
        out_data.push_back(color.g);
        out_data.push_back(color.b);
        out_data.push_back(uv.x);         // 6-7: Coordenada de textura
        out_data.push_back(uv.y);
        out_data.push_back(normal.x);     // 8-10: Normal
        out_data.push_back(normal.y);
        out_data.push_back(normal.z);

        indices.push_back(i);
    }

    if (out_data.empty())
    {
        printf("No vertices were generated from OBJ file.\n");
        return -1;
    }

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);  
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, out_data.size() * sizeof(float), out_data.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    const int stride = 11 * sizeof(GLfloat);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    numVertices = indices.size();  
    return VAO;
}

GLuint loadTexture(string path, int &width, int &height) {
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int nChannels;

	unsigned char *data = stbi_load(path.c_str(), &width, &height, &nChannels, 0);

	if (data) {
		if (nChannels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
        std::cout << "Failed to load texture " << path <<
        std::endl;
    }

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}