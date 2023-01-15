#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stb_image_write.h>

#include "Scene.h"

const std::string modelRelPath = "obj/bunny.obj";
const std::string outputPath = "screenshot.png";

Scene::Scene(const Options& options) : Application(options) {
	// init model
	_bunny.reset(new Model(getAssetFullPath(modelRelPath)));

	// init materials
	_material.reset(new Material);
	_material->ka = glm::vec3(0.03f, 0.03f, 0.03f);
	_material->kd = glm::vec3(1.0f, 1.0f, 1.0f);
	_material->ks = glm::vec3(1.0f, 1.0f, 1.0f);
	_material->ns = 10.0f;

	// init shaders
	initShader();

	// init lights
	_ambientLight.reset(new AmbientLight);

	_directionalLight.reset(new DirectionalLight);
	_directionalLight->intensity = 0.5f;
	_directionalLight->transform.rotation = glm::angleAxis(glm::radians(45.0f), glm::normalize(glm::vec3(-1.0f)));

	_spotLight.reset(new SpotLight);
	_spotLight->intensity = 0.5f;
	_spotLight->transform.position = glm::vec3(0.0f, 0.0f, 5.0f);
	_spotLight->transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	// init camera
	_camera.reset(new PerspectiveCamera(glm::radians(50.0f), 1.0f * _windowWidth / _windowHeight, 0.1f, 10000.0f));
	_camera->transform.position.z = 10.0f;

	// init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init();
}

Scene::~Scene() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Scene::initShader() {
	const char* vsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoord;\n"

		"out vec3 fPosition;\n"
		"out vec3 fNormal;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"

		"void main() {\n"
		"	fPosition = vec3(model * vec4(aPosition, 1.0f));\n"
		"	fNormal = mat3(transpose(inverse(model))) * aNormal;\n"
		"	// worldPosition = vec3(model * vec4(aPosition, 1.0f));\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";


	const char *fsCode =
		"#version 330 core\n"
		"in vec3 fPosition;\n"
		"in vec3 fNormal;\n"
		"out vec4 color;\n"

		"// material data structure declaration\n"
		"struct Material {\n"
		"	vec3 ka;\n"
		"	vec3 kd;\n"
		"	vec3 ks;\n"
		"	float ns;\n"
		"};\n"

		"// ambient light data structure declaration\n"
		"struct AmbientLight {\n"
		"	vec3 color;\n"
		"	float intensity;\n"
		"};\n"

		"// directional light data structure declaration\n"
		"struct DirectionalLight {\n"
		"	vec3 direction;\n"
		"	float intensity;\n"
		"	vec3 color;\n"
		"};\n"

		"// spot light data structure declaration\n"
		"struct SpotLight {\n"
		"	vec3 position;\n"
		"	vec3 direction;\n"
		"	float intensity;\n"
		"	vec3 color;\n"
		"	float angle;\n"
		"	float kc;\n"
		"	float kl;\n"
		"	float kq;\n"
		"};\n"

		"// uniform variables\n"
		"uniform vec3 viewPos;\n"
		"uniform Material material;\n"
		"uniform AmbientLight ambientLight;\n"
		"uniform DirectionalLight directionalLight;\n"
		"uniform SpotLight spotLight;\n"

		"vec3 calcDirectionalLight(vec3 normal, vec3 viewDir) {\n"
		"	vec3 lightDir = normalize(-directionalLight.direction);\n"
		"	vec3 diffuse = directionalLight.color * max(dot(lightDir, normal), 0.0f) * material.kd;\n"

		"	vec3 redirectDir = reflect(-lightDir, normal);\n"
		"	float spe = pow(max(dot(redirectDir, viewDir), 0), material.ns);\n"
		"	vec3 speculer = directionalLight.color * spe * material.ks;\n"
		"	vec3 result = speculer + diffuse;\n"
		"	return directionalLight.intensity * result ;\n"

		"}\n"

		"vec3 calcSpotLight(vec3 normal, vec3 viewDir) {\n"
		"	vec3 lightDir = normalize(spotLight.position - fPosition);\n"
		"	float theta = acos(-dot(lightDir, normalize(spotLight.direction)));\n"
		"	if (theta > spotLight.angle) {\n"
		"		return vec3(0.0f, 0.0f, 0.0f);\n"
		"	}\n"
		"	vec3 diffuse = spotLight.color * max(dot(lightDir, normal), 0.0f) * material.kd;\n"

		"	vec3 redirectDir = reflect(-lightDir, normal);\n"
		"	float spe = pow(max(dot(redirectDir, viewDir), 0), material.ns);\n"
		"	vec3 speculer = spotLight.color * spe * material.ks;\n"
		"	vec3 result = speculer + diffuse;\n"

		"	float distance = length(spotLight.position - fPosition);\n"
		"	float attenuation = 1.0f / (spotLight.kc + spotLight.kl * distance + spotLight.kq * distance * distance);\n"
		"	return spotLight.intensity * attenuation * result;\n"
		"}\n"

		"void main() {\n"
		"	vec3 normal = normalize(fNormal);\n"
		"	vec3 viewDir = normalize(viewPos - fPosition);\n"
		"	vec3 result = calcDirectionalLight(normal, viewDir) + calcSpotLight(normal, viewDir);\n"
		"	vec3 ambient = material.ka * ambientLight.color * ambientLight.intensity;\n"
		" 	result += ambient;\n"
		"	color = vec4(result, 1.0f);\n"
		"}\n";
	// ------------------------------------------------------------

	_Shader.reset(new GLSLProgram);
	_Shader->attachVertexShader(vsCode);
	_Shader->attachFragmentShader(fsCode);
	_Shader->link();
}

void Scene::handleInput() {
	constexpr float cameraMoveSpeed = 0.05f;
	constexpr float cameraRotateSpeed = 0.02f;

	if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		glfwSetWindowShouldClose(_window, true);
		return ;
	}
	
	// change mousemode
	if (_input.keyboard.keyStates[GLFW_KEY_LEFT_ALT] != GLFW_RELEASE) {
		_mouseMode = MouseMode::CameraMode;
		// set input mode to camera
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		_input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
		_input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
		glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);
	}
	if (_input.keyboard.keyStates[GLFW_KEY_RIGHT_ALT] != GLFW_RELEASE) {
		_mouseMode = MouseMode::GUIMode;
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// GameController::firstMouse = true;
	}

	// screen shot use key_u 
	if (_input.keyboard.keyStates[GLFW_KEY_U] != GLFW_RELEASE) {
		std::cout << "U" << std::endl;

		stbi_flip_vertically_on_write(true);
		// choose image format
		GLenum _format = GL_RGB;
		int _channels = 3;
		unsigned char *pixels = new unsigned char[_windowWidth * _windowHeight * _channels];
		// read pixels data
		glReadPixels(0, 0, _windowWidth, _windowHeight, _format, GL_UNSIGNED_BYTE, pixels);

		// write to png
		stbi_write_png(outputPath.c_str(), _windowWidth, _windowHeight, _channels, pixels, 0);

		delete[] pixels;
	}

	if(_mouseMode == MouseMode::CameraMode){
		glm::vec3 cameraFront = _camera->transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 cameraRight = _camera->transform.rotation * glm::vec3(1.0f, 0.0f, 0.0f);

		if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
			std::cout << "W" << std::endl;
			// move the camera in its front direction
			_camera->transform.position += cameraFront * cameraMoveSpeed;
		}

		if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
			std::cout << "A" << std::endl;
			// move the camera in its left direction
			_camera->transform.position -= cameraRight * cameraMoveSpeed;
		}

		if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
			std::cout << "S" << std::endl;
			// move the camera in its back direction
			_camera->transform.position -= cameraFront * cameraMoveSpeed;
		}

		if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
			std::cout << "D" << std::endl;
			// move the camera in its right direction
			_camera->transform.position += cameraRight * cameraMoveSpeed;
		}

		float xoffset;
		// static float yaw = 0;
		float yoffset;
		// static float pitch = 0;
		float rotateAngle;

		if (_input.mouse.move.xNow != _input.mouse.move.xOld) {
			std::cout << "mouse move in x direction" << std::endl;
			// rotate the camera around world up: glm::vec3(0.0f, 1.0f, 0.0f)
			xoffset = _input.mouse.move.xNow - _input.mouse.move.xOld;
			xoffset *= cameraRotateSpeed;
			rotateAngle = glm::radians(xoffset);

			// 绝对于y轴旋转 注意是绝对的
			_camera->transform.rotation = glm::quat(-rotateAngle * cameraUp) * _camera->transform.rotation;
		}

		if (_input.mouse.move.yNow != _input.mouse.move.yOld) {
			std::cout << "mouse move in y direction" << std::endl;
			// rotate the camera around its local right
			yoffset = -_input.mouse.move.yNow + _input.mouse.move.yOld;
			yoffset *= cameraRotateSpeed;
			rotateAngle = glm::radians(yoffset);

			_camera->transform.rotation = glm::quat(rotateAngle * cameraRight) * _camera->transform.rotation;
		}

		_input.forwardState();
	}

}

void Scene::renderFrame() {
	showFpsInWindowTitle();

	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	_Shader->use();
	// 1. transfer the mvp matrices to the shader
	_Shader->setUniformMat4("projection", _camera->getProjectionMatrix());
	_Shader->setUniformMat4("view", _camera->getViewMatrix());
	_Shader->setUniformMat4("model", _bunny->transform.getLocalMatrix());
	
	// 2. transfer the camera position to the shader
	_Shader->setUniformVec3("viewPos", _camera->transform.position);
	
	// 3. transfer the material attributes to the shader
	_Shader->setUniformVec3("material.ka", _material->ka);
	_Shader->setUniformVec3("material.kd", _material->kd);
	_Shader->setUniformVec3("material.ks", _material->ks);
	_Shader->setUniformFloat("material.ns", _material->ns);

	// 4. transfer the light attributes to the shader
	_Shader->setUniformVec3("ambientLight.color", _ambientLight->color);
	_Shader->setUniformFloat("ambientLight.intensity", _ambientLight->intensity);
	_Shader->setUniformVec3("spotLight.position", _spotLight->transform.position);
	_Shader->setUniformVec3("spotLight.direction", _spotLight->transform.getFront());
	_Shader->setUniformFloat("spotLight.intensity", _spotLight->intensity);
	_Shader->setUniformVec3("spotLight.color", _spotLight->color);
	_Shader->setUniformFloat("spotLight.angle", _spotLight->angle);
	_Shader->setUniformFloat("spotLight.kc", _spotLight->kc);
	_Shader->setUniformFloat("spotLight.kl", _spotLight->kl);
	_Shader->setUniformFloat("spotLight.kq", _spotLight->kq);
	_Shader->setUniformVec3("directionalLight.direction", _directionalLight->transform.getFront());
	_Shader->setUniformFloat("directionalLight.intensity", _directionalLight->intensity);
	_Shader->setUniformVec3("directionalLight.color", _directionalLight->color);

	// draw the bunny
	_bunny->draw();

	// draw ui elements
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	const auto flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings;

	if (!ImGui::Begin("Control Panel", nullptr, flags)) {
		ImGui::End();
	} else {
		ImGui::Text("Light Change");
		ImGui::Separator();

		// ImGui::RadioButton("", (int*)&_renderMode, (int)(RenderMode::));
		ImGui::ColorEdit3("ka##3", (float*)&_material->ka);
		ImGui::ColorEdit3("kd##3", (float*)&_material->kd);
		ImGui::ColorEdit3("ks##3", (float*)&_material->ks);
		ImGui::SliderFloat("ns##3", &_material->ns, 1.0f, 50.0f);
		ImGui::NewLine();

		ImGui::Text("ambient light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity##1", &_ambientLight->intensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("color##1", (float*)&_ambientLight->color);
		ImGui::NewLine();

		ImGui::Text("directional light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity##2", &_directionalLight->intensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("color##2", (float*)&_directionalLight->color);
		ImGui::NewLine();

		ImGui::Text("spot light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity##3", &_spotLight->intensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("color##3", (float*)&_spotLight->color);
		ImGui::SliderFloat("angle##3", (float*)&_spotLight->angle, 0.0f, glm::radians(180.0f), "%f rad");
		ImGui::NewLine();

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}