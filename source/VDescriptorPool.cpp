#include "include/vDescriptorPool.h"
#include <volk/volk.h>
#include <span>
#include <vector>

void DescriptorPool::clear_descriptors(VkDevice device) {
	vkResetDescriptorPool(device, pool, 0);
}

void DescriptorPool::destroy_pool(VkDevice device) {
	vkDestroyDescriptorPool(device, pool, 0);
}

void DescriptorPool::init_pool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
	
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (PoolSizeRatio ratio : poolRatios) {
		poolSizes.push_back(
			VkDescriptorPoolSize{
				.type = ratio.type,
				.descriptorCount = uint32_t(ratio.ratio * maxSets)
			});
	}
	

	VkDescriptorPoolCreateInfo pool_info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = 0,
		.maxSets = maxSets,
		.poolSizeCount = (uint32_t)poolSizes.size(),
		.pPoolSizes = poolSizes.data()
	};

	vkCreateDescriptorPool(device, &pool_info, nullptr, &pool);
}