#pragma once
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

#include <ktx.h>
#include <ktxvulkan.h>
#include <tiny_obj_loader.h>
#include "swapChain.h"
#include "vPipeline.h"
#include "vRenderer.h"

class VulkanApplication {
public:
    appCore aCore = {};

    vPipeline pipeline;
    SwapChain swapChain;

    bool updateSwapchain{ false };
    
    VkImage depthImage;
    VRenderer vRenderer;
  
    VmaAllocation depthImageAllocation;
    VkImageView depthImageView;
 
    VmaAllocation vBufferAllocation{ VK_NULL_HANDLE };
    VkBuffer vBuffer{ VK_NULL_HANDLE };
   
    
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    void init(int argc, char* argv[]);

    void initWindow();

    void createVulkanMemoryAllocator();

    void createDevice();

    void initVulkanCore(int argc, char* argv[]);

    void initVulkan(int argc, char* argv[]);

   // void createCommandsAndSync();

   // void drawFrame();

   
};