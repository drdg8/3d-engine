#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <glad/glad.h>

#include "bounding_box.h"
#include "transform.h"
#include "vertex.h"

class Sphere {
public:
    Sphere(const std::string& filepath);

    Sphere(glm::vec3 position, float r);

    Sphere(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    Sphere(Sphere&& rhs) noexcept;

    void addVertex(glm::vec3 position);

    glm::vec3 calNormals(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2);

    virtual ~Sphere();

    GLuint getVao() const;

    GLuint getBoundingBoxVao() const;

    size_t getVertexCount() const;

    size_t getFaceCount() const;

    BoundingBox getBoundingBox() const;

    virtual void draw() const;

    virtual void drawBoundingBox() const;

public:
    Transform transform;

protected:
    // vertices of the table represented in Sphere's own coordinate
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    std::unordered_map<Vertex, uint32_t> _uniqueVertices;

    // bounding box
    BoundingBox _boundingBox;

    // opengl objects
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

    GLuint _boxVao = 0;
    GLuint _boxVbo = 0;
    GLuint _boxEbo = 0;

    void computeBoundingBox();

    void initGLResources();

    void initBoxGLResources();

    void cleanup();
};