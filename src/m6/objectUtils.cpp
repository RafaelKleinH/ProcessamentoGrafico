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

GLuint loadTexture(string path, int &width, int &height)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int nChannels;

    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nChannels, 0);

    if (data)
    {
        if (nChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture " << path << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

int setupShader(GLchar *vertexShaderSource, GLchar *fragmentShaderSource)
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
        return -1; // Erro: arquivo não encontrado
    }

    std::string mtlFilename;
    std::string activeMaterial;

    while (1)
    {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
        {
            break; // Fim do arquivo
        }

        if (strcmp(lineHeader, "mtllib") == 0)
        {
            fscanf(file, "%127s\n", lineHeader);
            mtlFilename = lineHeader;
        }
        else if (strcmp(lineHeader, "usemtl") == 0)
        {
            fscanf(file, "%127s\n", lineHeader);
            activeMaterial = lineHeader;
        }
        else if (strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = 1.0f - uv.y;
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

        out_data.push_back(pos.x); // 0-2: Posição
        out_data.push_back(pos.y);
        out_data.push_back(pos.z);
        out_data.push_back(color.r); // 3-5: Cor
        out_data.push_back(color.g);
        out_data.push_back(color.b);
        out_data.push_back(uv.x); // 6-7: Coordenada de textura
        out_data.push_back(uv.y);
        out_data.push_back(normal.x); // 8-10: Normal
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

bool loadMTL(const std::string &directory, const std::string &mtlFilename, const std::string &activeMaterial, glm::vec3 &Ka, glm::vec3 &Kd, glm::vec3 &Ks, float &q, std::string &diffuseMapPath)
{
    std::string mtlPath = directory + "/" + mtlFilename;
    FILE *file = fopen(mtlPath.c_str(), "r");
    if (file == NULL)
    {
        printf("MTL file not found: %s\n", mtlPath.c_str());
        return false; // Arquivo não encontrado
    }

    bool foundMaterial = false;
    std::string currentMaterial;
    while (true)
    {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;
        if (strcmp(lineHeader, "newmtl") == 0)
        {
            fscanf(file, "%127s\n", lineHeader);
            currentMaterial = lineHeader;

            if (currentMaterial == activeMaterial)
                foundMaterial = true;
        }
        else if (foundMaterial)
        {
            if (strcmp(lineHeader, "Ka") == 0)
            {
                fscanf(file, "%f %f %f\n", &Ka.x, &Ka.y, &Ka.z);
            }
            else if (strcmp(lineHeader, "Kd") == 0)
            {
                fscanf(file, "%f %f %f\n", &Kd.x, &Kd.y, &Kd.z);
            }
            else if (strcmp(lineHeader, "Ks") == 0)
            {
                fscanf(file, "%f %f %f\n", &Ks.x, &Ks.y, &Ks.z);
            }
            else if (strcmp(lineHeader, "Ns") == 0)
            {
                fscanf(file, "%f\n", &q);
            }
            else if (strcmp(lineHeader, "map_Kd") == 0)
            {
                char mapPath[256];
                fscanf(file, "%255s\n", mapPath);
                diffuseMapPath = directory + "/" + mapPath;
            }
            else if (strcmp(lineHeader, "newmtl") == 0)
            {
                break;
            }
            else
            {
                char buffer[256];
                fgets(buffer, sizeof(buffer), file);
            }
        }
        else
        {
            char buffer[256];
            fgets(buffer, sizeof(buffer), file);
        }
    }

    fclose(file);
    return foundMaterial;
}





