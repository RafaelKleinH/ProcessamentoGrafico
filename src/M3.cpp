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
    //bool selected;
    glm::mat4 model;
};

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
float verifyValue(float value);
// Protótipos das funções
int setupShader();
int loadObj(const char *path, int &numVertices);

GLuint loadTexture(string path, int &width, int &height);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1920, HEIGHT = 1080;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = "#version 450\n"
                                   "layout (location = 0) in vec3 position;\n"
                                   "layout (location = 1) in vec3 vertexColor;\n"
                                   "layout (location = 2) in vec2 tex_coord;\n"

                                   "uniform mat4 model;\n"
                                   "out vec3 Color;\n"
                                   "out vec2 TexCoord;\n"
                                   "\n"
                                   "void main()\n"
                                   "{\n"
                                        "gl_Position =  model * vec4(position, 1.0);\n"
                                        "Color = vertexColor;\n"
                                        "TexCoord = tex_coord;\n"
                                   "}\0";

// Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = "#version 450\n"
                                    "\n"
                                    "in vec3 Color;\n"
                                    "in vec2 TexCoord;\n"
                                    "\n"
                                    "out vec4 color;\n"
                                    "uniform vec4 objectColor;\n"
                                    "uniform sampler2D tex_buffer;\n"
                                    "\n"
                                    "void main()\n"
                                    "{\n"
                                        "vec4 texColor = texture(tex_buffer, TexCoord);\n"
                                        "color = texColor * vec4(Color * objectColor.rgb, objectColor.a);\n"
                                    "}\n\0";

bool rotateX = false, rotateY = false, rotateZ = false, translateXPlus = false, translateXMinus = false, translateYPlus = false, translateYMinus = false, translateZPlus = false, translateZMinus = false, scalePlus = false, scaleMinus = false;
int principalObject = 0;
std::vector<Object> objects;
int imgWidth = 1024, imgHeight = 1024;
GLuint textId;

// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();

    // Muita atenção aqui: alguns ambientes não aceitam essas configurações
    // Você deve adaptar para a versão do OpenGL suportada por sua placa
    // Sugestão: comente essas linhas de código para desobrir a versão e
    // depois atualize (por exemplo: 4.5 com 4 e 5)
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Essencial para computadores da Apple
    // #ifdef __APPLE__
    //	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // #endif

    // Criação da janela GLFW
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

    textId = loadTexture("../assets/tex/pixelWall.png", imgWidth, imgHeight);
    if (textId == 0)
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    // Obtendo as informações de versão
    const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
    const GLubyte *version = glGetString(GL_VERSION);   /* version as a string */
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilando e buildando o programa de shader
    GLuint shaderID = setupShader();

    // Gerando um buffer simples, com a geometria de um triângulo
    int nVertices;
    GLuint VAO = loadObj("../assets/Modelos3D/Suzanne.obj", nVertices);

    glUseProgram(shaderID);

     GLint modelLoc = glGetUniformLocation(shaderID, "model");
     GLint projectionLoc = glGetUniformLocation(shaderID, "projection");
     GLint colorLoc = glGetUniformLocation(shaderID, "objectColor");
     GLint texLoc = glGetUniformLocation(shaderID, "tex_buffer");
     glUniform1i(texLoc, 0);

    glEnable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textId);



    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);
        
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
            objects[i].model = glm::rotate(objects[i].model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            objects[i].model = glm::translate(objects[i].model, glm::vec3(objects[i].x, objects[i].y, objects[i].z));
            objects[i].model = glm::rotate(objects[i].model, objects[i].angleX, glm::vec3(1.0f, 0.0f, 0.0f));
            objects[i].model = glm::rotate(objects[i].model, objects[i].angleY, glm::vec3(0.0f, 1.0f, 0.0f));
            objects[i].model = glm::rotate(objects[i].model, objects[i].angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
            objects[i].model = glm::scale(objects[i].model, glm::vec3(objects[i].scaleFactor));
            
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objects[i].model));

            if (i == principalObject) {
                glUniform4f(colorLoc, 0.039, 0.812, 0.039, 1.0f);
            } else {
                glUniform4f(colorLoc, 0.51, 0.09, 0.09, 1.0f);
            }
         
            glDrawElements(GL_TRIANGLES, nVertices, GL_UNSIGNED_INT, 0);
            
        }
        glBindVertexArray(0);

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    glDeleteVertexArrays(1, &VAO);
    // Finaliza a execução da GLFW, limpando os recursos alocados por ela
    glfwTerminate();
    return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
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

    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS)
            translateXPlus = true;
        else if (action == GLFW_RELEASE)
            translateXPlus = false;
    }

    if (key == GLFW_KEY_D)
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
        //std::cout << principalObject << std::endl; 
    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS && getArrayLength > 0)
    {
        objects.pop_back();
        principalObject = objects.size() - 1;   
    }
}

int setupShader()
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Checando erros de compilação (exibição via log no terminal)
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Checando erros de compilação (exibição via log no terminal)
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // Linkando os shaders e criando o identificador do programa de shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Checando por erros de linkagem
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

int loadObj(const char *path, int &numVertices)
{
    // https://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> indices;

    std::vector<float> out_data;

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return -1;
    }
    while (1)
    {

        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
        {
            break; 
        }

        if (strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
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
        }

        else
        {
            char buffer[256];
            fgets(buffer, sizeof(buffer), file);
        }
    }

    fclose(file);

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
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // hardcoded vertex color
        glm::vec2 uv = (uvIndex > 0 && uvIndex <= temp_uvs.size()) ? temp_uvs[uvIndex - 1] : glm::vec2(0.0f, 0.0f);

        out_data.push_back(pos.x);
        out_data.push_back(pos.y);
        out_data.push_back(pos.z);
        out_data.push_back(color.r);
        out_data.push_back(color.g);
        out_data.push_back(color.b);
        out_data.push_back(uv.x);
        out_data.push_back(uv.y);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

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