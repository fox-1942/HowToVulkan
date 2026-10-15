#include <vulkan/vulkan.h>
#include <volk/volk.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <filesystem>
#include <vma/vk_mem_alloc.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"
#include <ktx.h>
#include <ktxvulkan.h>
#include <tiny_obj_loader.h>

class VulkanApplication {
public:

    static constexpr uint32_t maxFramesInFlight{ 2 };

    uint32_t imageIndex{ 0 };
    uint32_t frameIndex{ 0 };
    VkInstance instance{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };
    VkQueue queue{ VK_NULL_HANDLE };
    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    bool updateSwapchain{ false };
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
    VkImage depthImage;
    VmaAllocator allocator{ VK_NULL_HANDLE };
    VmaAllocation depthImageAllocation;
    VkImageView depthImageView;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;
    std::array<VkFence, maxFramesInFlight> fences;
    std::array<VkSemaphore, maxFramesInFlight> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderCompleteSemaphores;
    VmaAllocation vBufferAllocation{ VK_NULL_HANDLE };
    VkBuffer vBuffer{ VK_NULL_HANDLE };
    VkSurfaceCapabilitiesKHR surfaceCaps{ };
    std::vector<VkPhysicalDevice> devices;
    VkExtent2D swapchainExtent{ };

    SDL_Window* window = nullptr;
    glm::ivec2 windowSize{};
    uint32_t deviceIndex{ 0 };

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    void run(int argc, char* argv[]);
    void initWindow();

    void createSwapchain();

    void createVulkanMemoryAllocator();

    void createDevice();

    void initVulkanCore(int argc, char* argv[]);

    void initVulkan(int argc, char* argv[]);

    void createCommandsAndSync();

    void drawFrame();

    void createPipeline();
};