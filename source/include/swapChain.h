#pragma once
#include "helper.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <vma/vk_mem_alloc.h>

class SwapChain {
public:
	VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
	VkExtent2D swapchainExtent{ };

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	VkSurfaceKHR surface {VK_NULL_HANDLE};
	VkSurfaceCapabilitiesKHR surfaceCaps{ };

	void createSwapchain(appCore& aCore) {
		chk(SDL_Vulkan_CreateSurface(aCore.window, aCore.instance, nullptr, &surface));
		chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(aCore.devices[aCore.deviceIndex], surface, &surfaceCaps));
		swapchainExtent = surfaceCaps.currentExtent; 
		if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) {
			swapchainExtent = { .width = static_cast<uint32_t>(aCore.windowSize.x), .height = static_cast<uint32_t>(aCore.windowSize.y) };
		}
		VkSwapchainCreateInfoKHR swapchainCI{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = surface,
			.minImageCount = surfaceCaps.minImageCount,
			.imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
			.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
			.imageExtent{.width = swapchainExtent.width, .height = swapchainExtent.height },
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = VK_PRESENT_MODE_FIFO_KHR
		};
		chk(vkCreateSwapchainKHR(aCore.device, &swapchainCI, nullptr, &swapchain));

		uint32_t imageCount{ 0 };
		chk(vkGetSwapchainImagesKHR(aCore.device, swapchain, &imageCount, nullptr));
		swapchainImages.resize(imageCount);
		chk(vkGetSwapchainImagesKHR(aCore.device, swapchain, &imageCount, swapchainImages.data()));
		swapchainImageViews.resize(imageCount);
		for (auto i = 0; i < imageCount; i++) {
			VkImageViewCreateInfo viewCI{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = swapchainImages[i], .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = VK_FORMAT_B8G8R8A8_SRGB, .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 } };
			chk(vkCreateImageView(aCore.device, &viewCI, nullptr, &swapchainImageViews[i]));
		}
	}
};