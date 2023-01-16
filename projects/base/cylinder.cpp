#pragma once
#include <iostream>
#include <limits>
#include <unordered_map>
#include <algorithm>
#include <math.h>
#include "cylinder.h"
#define PI 3.1415926
void Cylinder::addVertex(glm::vec3 position) {
    Vertex vertex{};
    vertex.position = position;
    if (_uniqueVertices.count(vertex) == 0) {
        _uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
        _vertices.push_back(vertex);
    }
    _indices.push_back(_uniqueVertices[vertex]);
}
glm::vec3 Cylinder::calNormals(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
    glm::vec3 p01 = p0 - p1;
    glm::vec3 p02 = p0 - p2;
    glm::vec3 p = glm::cross(p01, p02);
    return p;
}

Cylinder::Cylinder(glm::vec3 position, float r, float h) {//rΪ����뾶��hΪ��

    std::string err;

    //std::vector<Vertex> vertices;
    //std::vector<uint32_t> indices;
    //std::unordered_map<Vertex, uint32_t> uniqueVertices;
    float next;
    for (float angle = -PI / 720.0; angle <= 2.0001 * PI; angle += PI / 360.0) {
        next = angle + PI / 360.0;
        if (next >= 2.0 * PI) next = 0;

        //��Բ������

        addVertex(position + glm::vec3(r * sin(angle), 0, r * cos(angle)));
        addVertex(position + glm::vec3(r * sin(angle), h, r * cos(angle)));
        addVertex(position + glm::vec3(r * sin(next), h, r * cos(next)));

        addVertex(position + glm::vec3(r * sin(angle), 0, r * cos(angle)));
        addVertex(position + glm::vec3(r * sin(next), 0, r * cos(next)));
        addVertex(position + glm::vec3(r * sin(next), h, r * cos(next)));

        //��Բ������

        addVertex(position + glm::vec3(0, 0, 0));
        addVertex(position + glm::vec3(r * sin(angle), 0, r * cos(angle)));
        addVertex(position + glm::vec3(r * sin(next), 0, r * cos(next)));

        addVertex(position + glm::vec3(0, h, 0));
        addVertex(position + glm::vec3(r * sin(angle), h, r * cos(angle)));
        addVertex(position + glm::vec3(r * sin(next), h, r * cos(next)));

    }

    // check if the vertex appeared before to reduce redundant data

//_vertices = vertices;

    for (int i = 0; i < _indices.size(); i = i + 3)
    {
        _vertices[_indices[i]].normal += calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);
        _vertices[_indices[i + 1]].normal += calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);
        _vertices[_indices[i + 2]].normal += calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);

        _vertices[_indices[i]].texCoord.x = 0.1;
        _vertices[_indices[i]].texCoord.y = 0.1;
        _vertices[_indices[i + 1]].texCoord.x = 0.4;
        _vertices[_indices[i + 1]].texCoord.y = 0.4;
        _vertices[_indices[i + 2]].texCoord.x = 0.8;
        _vertices[_indices[i + 2]].texCoord.y = 0.8;
    }

    for (int i = 0; i < _vertices.size(); i++)
    {
        _vertices[i].normal = glm::normalize(_vertices[i].normal);
    }

    //_indices = indices;

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }
}

Cylinder::Cylinder(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : _vertices(vertices), _indices(indices) {

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }
}

Cylinder::Cylinder(Cylinder&& rhs) noexcept
    : _vertices(std::move(rhs._vertices)),
    _indices(std::move(rhs._indices)),
    _boundingBox(std::move(rhs._boundingBox)),
    _vao(rhs._vao), _vbo(rhs._vbo), _ebo(rhs._ebo),
    _boxVao(rhs._boxVao), _boxVbo(rhs._boxVbo), _boxEbo(rhs._boxEbo) {
    _vao = 0;
    _vbo = 0;
    _ebo = 0;
    _boxVao = 0;
    _boxVbo = 0;
    _boxEbo = 0;
}

Cylinder::~Cylinder() {
    cleanup();
}

BoundingBox Cylinder::getBoundingBox() const {
    return _boundingBox;
}

void Cylinder::draw() const {
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cylinder::drawBoundingBox() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(_boxVao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

GLuint Cylinder::getVao() const {
    return _vao;
}

GLuint Cylinder::getBoundingBoxVao() const {
    return _boxVao;
}

size_t Cylinder::getVertexCount() const {
    return _vertices.size();
}

size_t Cylinder::getFaceCount() const {
    return _indices.size() / 3;
}

void Cylinder::initGLResources() {
    // create a vertex array object
    glGenVertexArrays(1, &_vao);
    // create a vertex buffer object
    glGenBuffers(1, &_vbo);
    // create a element array buffer
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(Vertex) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        _indices.size() * sizeof(uint32_t), _indices.data(), GL_STATIC_DRAW);

    // specify layout, size of a vertex, data type, normalize, sizeof vertex array, offset of the attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Cylinder::computeBoundingBox() {
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    for (const auto& v : _vertices) {
        minX = std::min(v.position.x, minX);
        minY = std::min(v.position.y, minY);
        minZ = std::min(v.position.z, minZ);
        maxX = std::max(v.position.x, maxX);
        maxY = std::max(v.position.y, maxY);
        maxZ = std::max(v.position.z, maxZ);
    }

    _boundingBox.min = glm::vec3(minX, minY, minZ);
    _boundingBox.max = glm::vec3(maxX, maxY, maxZ);
}

void Cylinder::initBoxGLResources() {
    std::vector<glm::vec3> boxVertices = {
        glm::vec3(_boundingBox.min.x, _boundingBox.min.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.min.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.max.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.max.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.min.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.min.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.max.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.max.y, _boundingBox.max.z),
    };

    std::vector<uint32_t> boxIndices = {
        0, 1,
        0, 2,
        0, 4,
        3, 1,
        3, 2,
        3, 7,
        5, 4,
        5, 1,
        5, 7,
        6, 4,
        6, 7,
        6, 2
    };

    glGenVertexArrays(1, &_boxVao);
    glGenBuffers(1, &_boxVbo);
    glGenBuffers(1, &_boxEbo);

    glBindVertexArray(_boxVao);
    glBindBuffer(GL_ARRAY_BUFFER, _boxVbo);
    glBufferData(GL_ARRAY_BUFFER, boxVertices.size() * sizeof(glm::vec3), boxVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _boxEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, boxIndices.size() * sizeof(uint32_t), boxIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Cylinder::cleanup() {
    if (_boxEbo) {
        glDeleteBuffers(1, &_boxEbo);
        _boxEbo = 0;
    }

    if (_boxVbo) {
        glDeleteBuffers(1, &_boxVbo);
        _boxVbo = 0;
    }

    if (_boxVao) {
        glDeleteVertexArrays(1, &_boxVao);
        _boxVao = 0;
    }

    if (_ebo != 0) {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }

    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}