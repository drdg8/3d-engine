#pragma once

#include <iostream>
#include <limits>
#include <unordered_map>
#include <algorithm>

#include "cube.h"

void Cube::addVertex(glm::vec3 position) {
    Vertex vertex{};
    vertex.position = position;
    if (_uniqueVertices.count(vertex) == 0) {
        _uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
        _vertices.push_back(vertex);
    }
    _indices.push_back(_uniqueVertices[vertex]);
}

glm::vec3 Cube::calNormals(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
    glm::vec3 p01 = p0 - p1;
    glm::vec3 p02 = p0 - p2;
    glm::vec3 p = glm::cross(p01, p02);
    return p;
}

Cube::Cube(glm::vec3 position, float a) {//a «±ﬂ≥§

    std::string err;

    //std::vector<Vertex> vertices;
    //std::vector<uint32_t> indices;
    //std::unordered_map<Vertex, uint32_t> uniqueVertices;

    addVertex(position + glm::vec3(-a / 2, -a / 2, -a / 2));
    addVertex(position + glm::vec3(-a / 2, a / 2, a / 2));
    addVertex(position + glm::vec3(-a / 2, a / 2, -a / 2));


    addVertex(position + glm::vec3(-a / 2, -a / 2, -a / 2));
    addVertex(position + glm::vec3(-a / 2, -a / 2, a / 2));
    addVertex(position + glm::vec3(-a / 2, a / 2, a / 2));


    addVertex(position + glm::vec3(-a / 2, -a / 2, -a / 2));
    addVertex(position + glm::vec3(a / 2, -a / 2, -a / 2));
    addVertex(position + glm::vec3(a / 2, -a / 2, a / 2));


    addVertex(position + glm::vec3(-a / 2, -a / 2, -a / 2));
    addVertex(position + glm::vec3(a / 2, -a / 2, a / 2));
    addVertex(position + glm::vec3(-a / 2, -a / 2, a / 2));



    addVertex(position + glm::vec3(-a / 2, -a / 2, -a / 2));
    addVertex(position + glm::vec3(a / 2, a / 2, -a / 2));
    addVertex(position + glm::vec3(a / 2, -a / 2, -a / 2));



    addVertex(position + glm::vec3(-a / 2, -a / 2, -a / 2));
    addVertex(position + glm::vec3(-a / 2, a / 2, -a / 2));
    addVertex(position + glm::vec3(a / 2, a / 2, -a / 2));


    addVertex(position + glm::vec3(a / 2, a / 2, a / 2));
    addVertex(position + glm::vec3(-a / 2, a / 2, a / 2));
    addVertex(position + glm::vec3(-a / 2, -a / 2, a / 2));




    addVertex(position + glm::vec3(a / 2, a / 2, a / 2));
    addVertex(position + glm::vec3(-a / 2, -a / 2, a / 2));
    addVertex(position + glm::vec3(a / 2, -a / 2, a / 2));



    addVertex(position + glm::vec3(a / 2, a / 2, a / 2));
    addVertex(position + glm::vec3(-a / 2, a / 2, -a / 2));
    addVertex(position + glm::vec3(-a / 2, a / 2, a / 2));



    addVertex(position + glm::vec3(a / 2, a / 2, a / 2));
    addVertex(position + glm::vec3(a / 2, a / 2, -a / 2));
    addVertex(position + glm::vec3(-a / 2, a / 2, -a / 2));


    addVertex(position + glm::vec3(a / 2, a / 2, a / 2));
    addVertex(position + glm::vec3(a / 2, -a / 2, a / 2));
    addVertex(position + glm::vec3(a / 2, -a / 2, -a / 2));



    addVertex(position + glm::vec3(a / 2, a / 2, a / 2));
    addVertex(position + glm::vec3(a / 2, -a / 2, -a / 2));
    addVertex(position + glm::vec3(a / 2, a / 2, -a / 2));




    for (int i = 0; i < _indices.size(); i = i + 3)
    {
        //_vertices[_indices[i]].normal += calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);
        //_vertices[_indices[i + 1]].normal += calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);
        //_vertices[_indices[i + 2]].normal += calNormals(_vertices[_indices[i]].position, _vertices[_indices[i + 1]].position, _vertices[_indices[i + 2]].position);

        _vertices[_indices[i]].texCoord.x = 0.5;
        _vertices[_indices[i]].texCoord.y = 0;
        _vertices[_indices[i + 1]].texCoord.x = 0;
        _vertices[_indices[i + 1]].texCoord.y = 1;
        _vertices[_indices[i + 2]].texCoord.x = 1;
        _vertices[_indices[i + 2]].texCoord.y = 1;
    }

    for (int i = 0; i < _vertices.size(); i++)
    {
        _vertices[i].normal = glm::normalize((float)2.0*(_vertices[i].position - position)/a);
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


Cube::~Cube() {
    cleanup();
}


void Cube::draw() const {
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cube::drawBoundingBox() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(_boxVao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}