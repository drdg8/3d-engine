#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <glad/glad.h>
#include "model.h"
#include "bounding_box.h"
#include "transform.h"
#include "vertex.h"

class Cube :public Model{
public:
    Cube(glm::vec3 position, float a);

    void addVertex(glm::vec3 position);
    glm::vec3 calNormals(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2);

    ~Cube();

    virtual void draw() const;

    virtual void drawBoundingBox() const;

public:
    Transform transform;

protected:
    std::unordered_map<Vertex, uint32_t> _uniqueVertices;
};