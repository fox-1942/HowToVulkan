#pragma once
#include <vulkan/vulkan.h>
#include "helper.h"

class vPipeline {
public:
    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

    ShaderProcessor shaderProcessor;

    void createPipeline(appCore& aCore);

};