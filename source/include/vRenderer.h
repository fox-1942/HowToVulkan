#pragma once
#include <vulkan/vulkan.h>
#include <ktx.h>
#include <ktxvulkan.h>

#include <vector>
#include <array>
#include "helper.h"
#include "swapChain.h"

class VRenderer {
	static constexpr uint32_t maxFramesInFlight{ 2 };

	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;
	std::array<VkFence, maxFramesInFlight> fences;
	std::array<VkSemaphore, maxFramesInFlight> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderCompleteSemaphores;

public:
	void createCommandsAndSync(appCore& aCore, SwapChain swapChain);
	void drawFrame(appCore& aCore, SwapChain swapChain, VkPipeline pipeline);
};