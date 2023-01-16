#pragma once
#include <iostream>
#include <limits>
#include <unordered_map>
#include <algorithm>
#include <math.h>
#include "cone.h"
#define PI 3.1415926
void Cone::addVertex(glm::vec3 position) {
    Vertex vertex{};
    vertex.position = position;
    if (_uniqueVertices.count(vertex) == 0) {
        _uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
        _vertices.push_back(vertex);
    }
    _indices.push_back(_uniqueVertices[vertex]);
}
glm::vec3 Cone::calNormals(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
    glm::vec3 p01 = p0 - p1;
    glm::vec3 p02 = p0 - p2;
    glm::vec3 p = glm::cross(p01, p02);
    return p;
}

Cone::Cone(glm::vec3 position, float r, float h) {//rÎªµ×Ãæ°ë¾¶£¬hÎª¸ß

    std::string err;

    //std::vector<Vertex> vertices;
    //std::vector<uint32_t> indices;
    //std::unordered_map<Vertex, uint32_t> uniqueVertices;
    float next;
    for (float angle = 0; angle <= 2 * PI; angle += PI / 180.0) {
        next = angle + PI / 180.0;
        if (next > 2 * PI) next = 0;

        //»­Ô²×¶²àÃæ

        addVertex(position + glm::vec3(0, h, 0));
        addVertex(position + glm::vec3(r * sin(angle), 0, r * cos(angle)));
        addVertex(position + glm::vec3(r * sin(next), 0, r * cos(next)));

        //»­Ô²×¶µ×Ãæ

        addVertex(position + glm::vec3(0, 0, 0));
        addVertex(position + glm::vec3(r * sin(angle), 0, r * cos(angle)));
        addVertex(position + glm::vec3(r * sin(next), 0, r * cos(next)));

    }

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

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }
}

Cone::~Cone() {
    cleanup();
}

void Cone::draw() const {
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cone::drawBoundingBox() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(_boxVao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}