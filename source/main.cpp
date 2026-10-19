#include "include/vulkanRenderer.h"

int main(int argc, char* argv[]) {
	VulkanApplication app;
	app.run(argc, argv);
	SwapChain swa;
	bool quit{ false };
	while (!quit) {
		for (SDL_Event event; SDL_PollEvent(&event);) {
			if (event.type == SDL_EVENT_QUIT) quit = true;
		}
		app.drawFrame();
	}
	vkDeviceWaitIdle(app.aCore.device);
	return 0;
}