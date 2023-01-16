#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include "bounding_box.h"
#include "transform.h"
#include "vertex.h"
#include"texture.h"
#include "texture2d.h"
#include "glsl_program.h"
#include"light.h"

struct SingleMaterial {
	glm::vec3 kd = { 1.0f,1.0f,1.0f };
	std::shared_ptr<Texture2D> mapKd;
};
class Model {
public:
	Model() {}

	Model(const std::string& filepath);

	Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	Model(Model&& rhs) noexcept;

	virtual ~Model();

	GLuint getVao() const;

	GLuint getBoundingBoxVao() const;

	size_t getVertexCount() const;

	size_t getFaceCount() const;

	BoundingBox getBoundingBox() const;

	virtual void draw() const;

	virtual void drawBoundingBox() const;

	const std::string GetRelPath(int index);

	void initMaterial(std::string FullPath);

	void initSingleShader();

	void GPUDateTransfer(glm::mat4 projection, glm::mat4 view, glm::vec3 dir_light, glm::vec3 col_light, float int_light);

	int GetSelected();

	void SetID(int id);

	void SetSelected(int s = 1);

	/*std::unique_ptr<SingleMaterial> GetMaterial();

	int GetCurTextIndex();*/

	int _cur_text_index = 0;
	//material
	std::unique_ptr<SingleMaterial> _material;
	void UpDateTexture(std::string FullPath);
public:
	Transform transform;

protected:
	//id
	int _id = -1;
	//selected or not
	int _selected = 0;//1=selected
	//texture
	std::string TextureRelPath[3];//这边设置了三个默认的纹理，之后可以根据不同体素id来设定

	//shader
	std::unique_ptr<GLSLProgram> _singleShader;
	const char* _vsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoord;\n"
		"out vec3 fPosition;\n"
		"out vec3 fNormal;\n"
		"out vec2 fTexCoord;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"

		"void main() {\n"
		"	fPosition = vec3(model * vec4(aPosition, 1.0f));\n"
		"	fNormal = mat3(transpose(inverse(model))) * aNormal;\n"
		"	fTexCoord = aTexCoord;\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";

	const char* _fsCode =
		"#version 330 core\n"
		"in vec3 fPosition;\n"
		"in vec3 fNormal;\n"
		"in vec2 fTexCoord;\n"
		"out vec4 color;\n"

		"struct DirectionalLight {\n"
		"	vec3 direction;\n"
		"	vec3 color;\n"
		"	float intensity;\n"
		"};\n"

		"struct Material {\n"
		"	vec3 kd;\n"
		"};\n"

		"uniform Material material;\n"
		"uniform DirectionalLight light;\n"
		"uniform sampler2D mapKd;\n"

		"vec3 calcDirectionalLight(vec3 normal, vec3 kd) {\n"
		"	vec3 lightDir = normalize(-light.direction);\n"
		"	vec3 diffuse = light.color * max(dot(lightDir, normal), 0.0f) * kd;\n"
		"	return light.intensity * diffuse ;\n"
		"}\n"

		"void main() {\n"
		"	vec3 normal = normalize(fNormal);\n"
		"   vec3 diffuse=calcDirectionalLight(normal,material.kd);\n"
		"   vec4 color0=vec4(diffuse,1.0f);\n"
		"   color =color0*texture(mapKd,fTexCoord);\n"
		"}\n";

	// vertices of the table represented in model's own coordinate
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

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