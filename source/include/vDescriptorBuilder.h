#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
class DescriptorLayoutBuilder {

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	void add_binding(uint32_t binding, VkDescriptorType type, int dCount, VkShaderStageFlags shaderStages);

	void clear();

	VkDescriptorSetLayout buildSetLayout(VkDevice device, void* pNext, VkDescriptorSetLayoutCreateFlags flags);
};