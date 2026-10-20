#include "include/vulkanApplication.h"

int main(int argc, char* argv[]) {
	VulkanApplication app;
	app.init(argc, argv);
	SwapChain swa;
	bool quit{ false };
	while (!quit) {
		for (SDL_Event event; SDL_PollEvent(&event);) {
			if (event.type == SDL_EVENT_QUIT) quit = true;
		}
		app.vRenderer.drawFrame(app.aCore, app.swapChain, app.pipeline.pipeline);
	}
	vkDeviceWaitIdle(app.aCore.device);
	return 0;
}