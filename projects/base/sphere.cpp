#pragma once
#include <iostream>
#include <limits>
#include <unordered_map>
#include <algorithm>
#include <math.h>
#include "sphere.h"
#define PI 3.1415926
void Sphere::addVertex(glm::vec3 position) {
    Vertex vertex{};
    vertex.position = position;
    if (_uniqueVertices.count(vertex) == 0) {
        _uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
        _vertices.push_back(vertex);
    }
    _indices.push_back(_uniqueVertices[vertex]);
}
glm::vec3 Sphere::calNormals(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
    glm::vec3 p01 = p0 - p1;
    glm::vec3 p02 = p0 - p2;
    glm::vec3 p = glm::cross(p01, p02);
    return p;
}

Sphere::Sphere(glm::vec3 position, float r) {//r为半径

    std::string err;

    //使用球坐标系
    float next_phi, next_theta;
    for (float phi = 0; phi <= 2 * PI; phi += PI / 360.0) {
        next_phi = phi + PI / 360.0;
        if (next_phi > PI)next_phi = PI;

        for (float theta = -PI / 720.0; theta <= 2.0 * PI; theta += PI / 360.0) {
            next_theta = theta + PI / 360.0;
            if (next_theta >= 2.0 * PI) next_theta = 0;

            addVertex(position + glm::vec3(r * sin(phi) * cos(theta), r * cos(phi), r * sin(phi) * sin(theta)));
            addVertex(position + glm::vec3(r * sin(next_phi) * cos(theta), r * cos(next_phi), r * sin(next_phi) * sin(theta)));
            addVertex(position + glm::vec3(r * sin(next_phi) * cos(next_theta), r * cos(next_phi), r * sin(next_phi) * sin(next_theta)));

            addVertex(position + glm::vec3(r * sin(phi) * cos(theta), r * cos(phi), r * sin(phi) * sin(theta)));
            addVertex(position + glm::vec3(r * sin(phi) * cos(next_theta), r * cos(phi), r * sin(phi) * sin(next_theta)));
            addVertex(position + glm::vec3(r * sin(next_phi) * cos(next_theta), r * cos(next_phi), r * sin(next_phi) * sin(next_theta)));

        }
    }

    for (int i = 0; i < _indices.size(); i = i + 3)
    {
        _vertices[_indices[i]].normal -= calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);
        _vertices[_indices[i + 1]].normal -= calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);
        _vertices[_indices[i + 2]].normal -= calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);

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

Sphere::~Sphere() {
    cleanup();
}

void Sphere::draw() const {
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sphere::drawBoundingBox() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(_boxVao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}