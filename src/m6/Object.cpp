// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BezierPoint
{
    float x, y, z;
};

struct Object
{
    int id;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
    float scaleFactor = 1.0f;    
    glm::mat4 model;
    
    std::vector<BezierPoint> path;
    int pathIndex = 0;
    bool pathFollow = false;
};