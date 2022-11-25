#include <iostream>
#include <cstdlib>

#include "Scene.h"

Options getOptions(int argc, char* argv[]) {
	Options options;
	options.windowTitle = "Scene";
	options.windowWidth = 1280;
	options.windowHeight = 720;
	options.windowResizable = false;
	options.vSync = true;
	options.msaa = true;
	options.glVersion = { 3, 3 };
	options.backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	options.assetRootDir = "../../media/";

	return options;
}

int main(int argc, char* argv[]) {
	Options options = getOptions(argc, argv);

	try {
		Scene app(options);
		app.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		while (1) { /* code */ }
		return EXIT_FAILURE;
	} catch (...) {
		std::cerr << "Unknown Error" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}