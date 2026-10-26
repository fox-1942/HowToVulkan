#pragma once
#include <volk/volk.h>
#include <span>
#include <vector>
class DescriptorPool {

	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};


	VkDescriptorPool pool{};

	void clear_descriptors(VkDevice device);
	void destroy_pool(VkDevice device);
	void init_pool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
};