#pragma once

#include <memory>
#include <string>

#include <imgui.h>

#include "../base/application.h"
#include "../base/glsl_program.h"
#include "../base/model.h"
#include "../base/light.h"
#include "../base/camera.h"
// I = ka * Ia + sum { (kd * cos(theta[i]) + ks * cos(theta[i])^ns ) * Light[i] }
struct Material {
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	float ns;
};

class Scene : public Application {
public:
	Scene(const Options& options);

	~Scene();
private:
	// model
	std::unique_ptr<Model> _bunny;

	// materials
	std::unique_ptr<Material> _material;

	// shaders
	std::unique_ptr<GLSLProgram> _Shader;

	// lights
	std::unique_ptr<AmbientLight> _ambientLight;
	std::unique_ptr<DirectionalLight> _directionalLight;
	std::unique_ptr<SpotLight> _spotLight;

	// camera
	std::unique_ptr<PerspectiveCamera> _camera;

	// I = ka * albedo + kd * cos<I, n> + ks * (max(cos<R, V>, 0) ^ ns)
	void initShader();

	void handleInput() override;

	void renderFrame() override;
};