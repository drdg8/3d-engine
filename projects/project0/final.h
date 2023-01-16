#pragma once

#include <memory>
#include <string>

#include "../base/application.h"
#include "../base/model.h"//这个model是自己修改过的
#include "../base/light.h"
#include "../base/glsl_program.h"
#include "../base/texture.h"
#include "../base/camera.h"
#include "../base/skybox.h"

enum class RenderMode {
	Create,Select, Others
};

class Final : public Application {
public:
	Final(const Options& options);

	~Final();

private:
	enum RenderMode _renderMode = RenderMode::Others;

	void handleInput() override;

	void renderFrame() override;

	std::unique_ptr<PerspectiveCamera> _camera;
	std::unique_ptr<DirectionalLight> _light;

	int _id_counter = 0;
	float _PositionMoveSpeed = 0.5f;
	float _angluarVelocity = 0.1f;
	float _scaleSpeed = 0.1f;
	std::vector<std::unique_ptr<Model>> _objs;
};