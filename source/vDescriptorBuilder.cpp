#include "include/vDescriptorBuilder.h"
#include <volk/volk.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>

void DescriptorLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType type, int dCount, VkShaderStageFlags shaderStages) {
	VkDescriptorSetLayoutBinding layoutBinding{
		.binding = binding,
		.descriptorType = type,
		.descriptorCount = dCount,
		.stageFlags = shaderStages
	};

	bindings.push_back(layoutBinding);
}

void DescriptorLayoutBuilder::clear()
{
	bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::buildSetLayout(VkDevice device, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
{
	VkDescriptorSetLayoutCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.bindingCount = bindings.size(),
		.pBindings = bindings.data()
	};

	VkDescriptorSetLayout set;
	vkCreateDescriptorSetLayout(device, &info, nullptr, &set);

	return set;
}

