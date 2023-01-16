#include <iostream>
#include <limits>
#include <unordered_map>
#include <algorithm>

#include <tiny_obj_loader.h>

#include "model.h"


Model::Model(const std::string& filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;

    std::string::size_type index = filepath.find_last_of("/");
    std::string mtlBaseDir = filepath.substr(0, index + 1);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str(), mtlBaseDir.c_str())) {
        throw std::runtime_error("load " + filepath + " failure: " + err);
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
            vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
            vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];

            if (index.normal_index >= 0) {
                vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
                vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
                vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
            }

            if (index.texcoord_index >= 0) {
                vertex.texCoord.x = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.texCoord.y = attrib.texcoords[2 * index.texcoord_index + 1];
            }

            // check if the vertex appeared before to reduce redundant data
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    _vertices = vertices;
    _indices = indices;

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }

    std::string str("texture/miscellaneous/planet_Quom1200.png");
    std::string str1("texture/miscellaneous/earthmap.jpg");
    for (int i = 0; i < 3; i++)
        TextureRelPath[i] = str1;
    TextureRelPath[1] = str;

}

Model::Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : _vertices(vertices), _indices(indices) {

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }
    std::string str("texture/miscellaneous/planet_Quom1200.png");
    std::string str1("texture/miscellaneous/earthmap.jpg");
    for (int i = 0; i < 3; i++)
        TextureRelPath[i] = str1;
    TextureRelPath[1] = str;
}

Model::Model(Model&& rhs) noexcept
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
    std::string str("texture/miscellaneous/planet_Quom1200.png");
    std::string str1("texture/miscellaneous/earthmap.jpg");
    for (int i = 0; i < 3; i++)
        TextureRelPath[i] = str1;
    TextureRelPath[1] = str;
}

Model::~Model() {
    cleanup();
}

BoundingBox Model::getBoundingBox() const {
    return _boundingBox;
}

void Model::draw() const {
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Model::drawBoundingBox() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(_boxVao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

GLuint Model::getVao() const {
    return _vao;
}

GLuint Model::getBoundingBoxVao() const {
    return _boxVao;
}

size_t Model::getVertexCount() const {
    return _vertices.size();
}

size_t Model::getFaceCount() const {
    return _indices.size() / 3;
}

void Model::initGLResources() {
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

void Model::computeBoundingBox() {
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

void Model::initBoxGLResources() {
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

void Model::cleanup() {
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

const std::string Model::GetRelPath(int index) {
    if (index < 0 || index>2)
        index = 0;
    std::string RelPath = TextureRelPath[index];
    return RelPath;
}

void Model::initMaterial(std::string FullPath)
{
    std::shared_ptr<Texture2D> Texture =
        std::make_shared<Texture2D>(FullPath);

    _material.reset(new SingleMaterial);
    _material->mapKd = Texture;
}

void Model::initSingleShader() {
    _singleShader.reset(new GLSLProgram);
    _singleShader->attachVertexShader(_vsCode);
    _singleShader->attachFragmentShader(_fsCode);
    _singleShader->link();
}

void Model::GPUDateTransfer(glm::mat4 projection, glm::mat4 view, glm::vec3 dir_light, glm::vec3 col_light, float int_light) {
    // 1. use the shader
    _singleShader->use();
    //std::cout << "307" << std::endl;
    // 2. transfer mvp matrices to gpu 
    _singleShader->setUniformMat4("projection", projection);
    //std::cout << "310" << std::endl;
    _singleShader->setUniformMat4("view", view);
    //std::cout << "312" << std::endl;
    _singleShader->setUniformMat4("model", transform.getLocalMatrix());
    //std::cout << "314" << std::endl;
    // 3. transfer light attributes to gpu
    _singleShader->setUniformVec3("light.direction", dir_light);
    //std::cout << "317" << std::endl;
    _singleShader->setUniformVec3("light.color", col_light);
    //std::cout << "319" << std::endl;
    _singleShader->setUniformFloat("light.intensity", int_light);
    //std::cout << "321" << std::endl;
    // 4. transfer materials to gpu
    _singleShader->setUniformVec3("material.kd", _material->kd);
    //std::cout << "325" << std::endl;
    // 4.3 TODO: enable textures and transform textures to gpu
    // write your code here
    //----------------------------------------------------------------
    glActiveTexture(GL_TEXTURE0);
    //std::cout << "329" << std::endl;
    _material->mapKd->bind(0);
    //std::cout << "331" << std::endl;
    _singleShader->setUniformInt("mapKd", 0);
    //std::cout << "333" << std::endl;
    //----------------------------------------------------------------
}

int Model::GetSelected()
{
    return _selected;
}

void Model::SetID(int id)
{
    _id = id;
}

void Model::SetSelected(int s)
{
    _selected = s;
}

/*std::unique_ptr<SingleMaterial> Model::GetMaterial() {
    return std::move(_material);
}

int Model::GetCurTextIndex() {
    return _cur_text_index;
}*/

void Model::UpDateTexture(std::string FullPath) {
   std::shared_ptr<Texture2D> Texture =
        std::make_shared<Texture2D>(FullPath);

   _material->mapKd = Texture;
}