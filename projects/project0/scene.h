#pragma once

#include <memory>
#include <string>

#include <imgui.h>

#include "../base/application.h"
#include "../base/glsl_program.h"
#include "../base/model.h"
#include "../base/light.h"
#include "../base/camera.h"
#include "../base/cube.h"
#include "../base/cone.h"
#include "../base/cylinder.h"
#include "../base/sphere.h"
// I = ka * Ia + sum { (kd * cos(theta[i]) + ks * cos(theta[i])^ns ) * Light[i] }
struct Material {
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	float ns;
};

enum class MouseMode {
	GUIMode, CameraMode
};

class Scene : public Application {
public:
	Scene(const Options& options);

	~Scene();
private:
	// model
	std::unique_ptr<Model> _bunny;
	Cube* _cube;
	Cone* _cone;
	Cylinder* _cylinder;
	Sphere* _sphere;

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

	// mouse
	enum MouseMode _mouseMode = MouseMode::GUIMode;

	// I = ka * albedo + kd * cos<I, n> + ks * (max(cos<R, V>, 0) ^ ns)
	void initShader();

	void handleInput() override;

	void renderFrame() override;
};