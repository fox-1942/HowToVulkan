#pragma once
#include <volk/volk.h>
#include <vulkan/vulkan.h>
#include <string>
#include <iostream>
#include <SDL3/SDL.h>
#include <fstream>
#include <sstream>
#include <slang/slang-com-ptr.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vma/vk_mem_alloc.h>

struct appCore {
	uint32_t imageIndex{ 0 };
	uint32_t frameIndex{ 0 };
	VkInstance instance{ VK_NULL_HANDLE };
	VkDevice device{ VK_NULL_HANDLE };
	VkQueue queue{ VK_NULL_HANDLE };
	SDL_Window* window = nullptr;
	glm::ivec2 windowSize{};
	uint32_t deviceIndex{ 0 };
	VmaAllocator allocator{ VK_NULL_HANDLE };
	std::vector<VkPhysicalDevice> devices;
};

static inline void chk(VkResult result) {
	if (result != VK_SUCCESS) {
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}

static inline void chk(bool result) {
	if (!result) {
		std::cerr << "Call returned an error\n";
		std::cout << SDL_GetError() << std::endl;
		exit(result);
	}
}

class ShaderProcessor {

public:

	std::string readUpShader(const std::string& filePath);

	VkShaderModule compileSlangToShaderModule(VkDevice device, const std::string& shaderCode, const char* entryPointName);
};
