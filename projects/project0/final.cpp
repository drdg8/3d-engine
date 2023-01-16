#include "final.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
const std::string modelRelPath = "obj/sphere.obj";

Final::Final(const Options& options) : Application(options) {
	// init camera
	_camera.reset(new PerspectiveCamera(
		glm::radians(50.0f), 1.0f * _windowWidth / _windowHeight, 0.1f, 10000.0f));
	_camera->transform.position.z = 10.0f;
	// init light
	_light.reset(new DirectionalLight());
	_light->transform.rotation =
		glm::angleAxis(glm::radians(45.0f), glm::normalize(glm::vec3(-1.0f, -2.0f, -1.0f)));

	// init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init();
}

Final::~Final() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}



void Final::handleInput() {
	if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		glfwSetWindowShouldClose(_window, true);
		return;
	}

	if (_renderMode == RenderMode::Create)//可以进行优化
	{
		std::vector<std::unique_ptr<Model>>::reverse_iterator it1 = _objs.rbegin();//末尾最新添加，两种情况最容易找到
		for (; it1 != _objs.rend(); ++it1)
		{
			if ((*it1)->GetSelected())
			{
				(*it1)->SetSelected(0);
				break;
			}
		}
		if (_input.keyboard.keyStates[GLFW_KEY_N] == GLFW_PRESS)
		{

			std::unique_ptr<Model>* Mypointer = (std::unique_ptr<Model> *)new(std::unique_ptr<Model>);

			Mypointer->reset(new Model(getAssetFullPath(modelRelPath)));

			(*Mypointer)->SetID(_id_counter++);

			_objs.push_back(std::move(*Mypointer));

			//init material
			//std::vector<std::unique_ptr<Model>>::iterator it = _objs.begin();
			//for (; it != _objs.end(); ++it)
			//{
			//	(*it)->initMaterial(getAssetFullPath((*it)->GetRelPath(0)));
			//}//不能这么做，相当于每次刷新

			//init shader
			//it = _objs.begin();
			//for (; it != _objs.end(); ++it)
			//{
				//(*it)->initSingleShader();//这个不能这么搞,相当于最后一个有效
			//}

			std::cout << "N" << std::endl;

			std::vector<std::unique_ptr<Model>>::reverse_iterator itr = _objs.rbegin();
			(*itr)->initMaterial(getAssetFullPath((*itr)->GetRelPath((*itr)->_cur_text_index)));
			(*itr)->SetSelected();
			_input.keyboard.keyStates[GLFW_KEY_N] = GLFW_RELEASE;
			_renderMode = RenderMode::Select;//保证N按动一次，只创建一个新体素
		}
	}

	if (_renderMode == RenderMode::Select)//可以进行优化
	{
		//if(_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS)
		int no_select_flag = 0;
		if (_objs.size())
		{
			std::vector<std::unique_ptr<Model>>::reverse_iterator it2 = _objs.rbegin();//末尾最新添加，两种情况最容易找到
			for (; it2 != _objs.rend(); ++it2)
			{
				if ((*it2)->GetSelected())
					break;
			}
			if (it2 == _objs.rend())
			{
				no_select_flag = 1;
			}


			if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS)
			{
				if (!no_select_flag)
					(*it2)->SetSelected(0);

				//此处不需要再修改space值，因为和连续按动时间无关
				_renderMode = RenderMode::Others;
			}
			if (_input.keyboard.keyStates[GLFW_KEY_LEFT_CONTROL] == GLFW_PRESS && _input.mouse.press.left)
			{
				int not_in_region_flag = 0;
				if (!no_select_flag)
				{
					(*it2)->SetSelected(0);
					not_in_region_flag = 0;
				}

				it2 = _objs.rbegin();

				float mousex = -2 * _input.mouse.move.xNow / _windowWidth + 1.0f;
				float mousey = -2 * _input.mouse.move.yNow / _windowHeight + 1.0f;
				std::cout << "Mx: " << mousex << std::endl;
				std::cout << "My: " << mousey << std::endl;
				int i = 0;
				glm::mat4 Myprojection = _camera->getProjectionMatrix();
				glm::mat4 Myview = _camera->getViewMatrix();
				for (; it2 != _objs.rend(); it2++)
				{
					//"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
					//"	fPosition = vec3(model * vec4(aPosition, 1.0f));\n"
					BoundingBox bbox = (*it2)->getBoundingBox();//需要计算transform后的bbx
					glm::vec4 MIN_BOX_gl = Myprojection * Myview * ((*it2)->transform.getLocalMatrix()) * glm::vec4(bbox.min, 1.0f);
					glm::vec4 MAX_BOX_gl = Myprojection * Myview * ((*it2)->transform.getLocalMatrix()) * glm::vec4(bbox.max, 1.0f);
					glm::vec3 MIN_BOX;
					MIN_BOX.x = -MIN_BOX_gl.x / MIN_BOX_gl.w;
					MIN_BOX.y = MIN_BOX_gl.y / MIN_BOX_gl.w;
					MIN_BOX.z = MIN_BOX_gl.z / MIN_BOX_gl.w;
					glm::vec3 MAX_BOX;
					MAX_BOX.x = -MAX_BOX_gl.x / MAX_BOX_gl.w;
					MAX_BOX.y = MAX_BOX_gl.y / MAX_BOX_gl.w;
					MAX_BOX.z = MAX_BOX_gl.z / MAX_BOX_gl.w;
					std::cout << "Model " << i << ": minx:" << MAX_BOX.x << "  maxx:" << MIN_BOX.x << std::endl;
					std::cout << "Model " << i << ": miny:" << MIN_BOX.y << "  maxy:" << MAX_BOX.y << std::endl;
					//std::cout << "posX: " << (*it2)->transform.position.x << " posY: " << (*it2)->transform.position.y << " posZ: " << (*it2)->transform.position.z << std::endl;
					if (mousex < MIN_BOX.x && mousey > MIN_BOX.y && mousex > MAX_BOX.x && mousey < MAX_BOX.y)
						break;
					i++;
				}
				if (it2 == _objs.rend())
				{
					not_in_region_flag = 1;
				}
				else
					not_in_region_flag = 0;
				if (!not_in_region_flag)
					(*it2)->SetSelected();
			}
			if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE && !no_select_flag) {

				//std::cout << "W" << std::endl;
				(*it2)->transform.position.y += _PositionMoveSpeed;
				// -------------------------------------------------
			}

			if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE && !no_select_flag) {
				//std::cout << "A" << std::endl;
				(*it2)->transform.position.x -= _PositionMoveSpeed;
			}

			if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE && !no_select_flag) {
				//std::cout << "S" << std::endl;
				(*it2)->transform.position.y -= _PositionMoveSpeed;
			}

			if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE && !no_select_flag) {
				//std::cout << "D" << std::endl;
				(*it2)->transform.position.x += _PositionMoveSpeed;
			}
			if (_input.keyboard.keyStates[GLFW_KEY_LEFT] != GLFW_RELEASE && !no_select_flag) {
				const float angle = _angluarVelocity;
				const glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
				(*it2)->transform.rotation = glm::angleAxis(angle, axis) * (*it2)->transform.rotation;
			}
			if (_input.keyboard.keyStates[GLFW_KEY_RIGHT] != GLFW_RELEASE && !no_select_flag) {
				const float angle = _angluarVelocity;
				const glm::vec3 axis = glm::vec3(0.0f, -1.0f, 0.0f);
				(*it2)->transform.rotation = glm::angleAxis(angle, axis) * (*it2)->transform.rotation;
			}
			if (_input.keyboard.keyStates[GLFW_KEY_UP] != GLFW_RELEASE && !no_select_flag) {
				const float angle = _angluarVelocity;
				const glm::vec3 axis = glm::vec3(1.0f, 0.0f, 0.0f);
				(*it2)->transform.rotation = glm::angleAxis(angle, axis) * (*it2)->transform.rotation;
			}
			if (_input.keyboard.keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE && !no_select_flag) {
				const float angle = _angluarVelocity;
				const glm::vec3 axis = glm::vec3(-1.0f, 0.0f, 0.0f);
				(*it2)->transform.rotation = glm::angleAxis(angle, axis) * (*it2)->transform.rotation;
			}
			if (_input.keyboard.keyStates[GLFW_KEY_LEFT_BRACKET] != GLFW_RELEASE && !no_select_flag) {
				float sca = (*it2)->transform.scale.x;
				if (sca > _scaleSpeed)
				{
					(*it2)->transform.scale.x -= _scaleSpeed;
					(*it2)->transform.scale.y -= _scaleSpeed;
					(*it2)->transform.scale.z -= _scaleSpeed;
				}
			}
			if (_input.keyboard.keyStates[GLFW_KEY_RIGHT_BRACKET] != GLFW_RELEASE && !no_select_flag) {
				(*it2)->transform.scale.x += _scaleSpeed;
				(*it2)->transform.scale.y += _scaleSpeed;
				(*it2)->transform.scale.z += _scaleSpeed;
			}
			/*if (_input.keyboard.keyStates[GLFW_KEY_BACKSPACE] != GLFW_RELEASE && !no_select_flag) {
				(*it2)->SetSelected(0);
				//_objs
				_renderMode = RenderMode::Others;
			}*/
		}
		else
		{
			_renderMode = RenderMode::Others;
			std::vector<std::unique_ptr<Model>>::reverse_iterator it3 = _objs.rbegin();//末尾最新添加，两种情况最容易找到
			for (; it3 != _objs.rend(); ++it3)
			{
				if ((*it3)->GetSelected())
				{
					(*it3)->SetSelected(0);
					break;
				}
			}
		}
	}
}

void Final::renderFrame() {
	showFpsInWindowTitle();

	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	const glm::mat4 projection = _camera->getProjectionMatrix();
	const glm::mat4 view = _camera->getViewMatrix();


	std::vector<std::unique_ptr<Model>>::reverse_iterator it = _objs.rbegin();
	for (; it != _objs.rend(); ++it)
	{
		//std::cout << "Draw Round " << ++i << std::endl;
		(*it)->UpDateTexture(getAssetFullPath((*it)->GetRelPath((*it)->_cur_text_index)));
		(*it)->initSingleShader();
		(*it)->GPUDateTransfer(projection, view, _light->transform.getFront(), _light->color, _light->intensity);
		(*it)->draw();
		if ((*it)->GetSelected())
		{
			(*it)->drawBoundingBox();
		}

	}
	// draw ui elements
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	const auto flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings;

	if (!ImGui::Begin("Control Panel", nullptr, flags)) {
		ImGui::End();
	}
	else {

		ImGui::RadioButton("Create              ", (int*)&_renderMode, (int)(RenderMode::Create));
		ImGui::NewLine();

		ImGui::RadioButton("Select              ", (int*)&_renderMode, (int)(RenderMode::Select));
		ImGui::NewLine();

		ImGui::RadioButton("Others", (int*)&_renderMode, (int)(RenderMode::Others));

		ImGui::Text("Directional light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity", &_light->intensity, 0.0f, 2.0f);
		ImGui::ColorEdit3("color", (float*)&_light->color);
		ImGui::NewLine();
		int no_select_flag = 0;
		std::vector<std::unique_ptr<Model>>::reverse_iterator it0 = _objs.rbegin();//末尾最新添加，两种情况最容易找到
		switch (_renderMode) {
		case RenderMode::Select:
			ImGui::Separator();
			ImGui::Text("Object Attributes");
			ImGui::Separator();
			no_select_flag = 0;
			if (_objs.size())
			{

				for (; it0 != _objs.rend(); ++it0)
				{
					if ((*it0)->GetSelected())
						break;
				}
				if (it0 == _objs.rend())
				{
					no_select_flag = 1;
				}
				else
					no_select_flag = 0;
			}
			else
				no_select_flag = 1;
			if (!no_select_flag)
			{
				ImGui::ColorEdit3("kd", (float*)&(*it0)->_material->kd);
				ImGui::SliderInt("texture index", &(*it0)->_cur_text_index, 0, 2);
			}
			break;
		case RenderMode::Others:
			break;
		case RenderMode::Create:
			break;
		}

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}