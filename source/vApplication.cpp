#define VMA_IMPLEMENTATION
#define VOLK_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include "include/vApplication.h"

void VulkanApplication::init(int argc, char* argv[]) {
	initWindow();
	initVulkan(argc, argv);
	vRenderer.createCommandsAndSync(aCore, swapChain);
}

void VulkanApplication::initWindow() {
	chk(SDL_Init(SDL_INIT_VIDEO) == true);
	aCore.window = SDL_CreateWindow("How to Vulkan", 1280u, 720u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	assert(aCore.window);
	chk(SDL_GetWindowSize(aCore.window, &aCore.windowSize.x, &aCore.windowSize.y));
}

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

void VulkanApplication::createVulkanMemoryAllocator() {
	VmaVulkanFunctions vkFunctions{ .vkGetInstanceProcAddr = vkGetInstanceProcAddr, .vkGetDeviceProcAddr = vkGetDeviceProcAddr, .vkCreateImage = vkCreateImage };
	VmaAllocatorCreateInfo allocatorCI{ .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, .physicalDevice = aCore.devices[aCore.deviceIndex], .device = aCore.device, .pVulkanFunctions = &vkFunctions, .instance = aCore.instance };
	chk(vmaCreateAllocator(&allocatorCI, &aCore.allocator));
}

void VulkanApplication::createDevice() {
	// Find a queue family for graphics
	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(aCore.devices[aCore.deviceIndex], &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(aCore.devices[aCore.deviceIndex], &queueFamilyCount, queueFamilies.data());
	uint32_t queueFamily{ 0 };
	for (size_t i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamily = i;
			break;
		}
	}
	chk(SDL_Vulkan_GetPresentationSupport(aCore.instance, aCore.devices[aCore.deviceIndex], queueFamily));

	// Logical device
	const float qfpriorities{ 1.0f };
	VkDeviceQueueCreateInfo queueCI{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = queueFamily, .queueCount = 1, .pQueuePriorities = &qfpriorities };

	VkPhysicalDeviceVulkan11Features enabledVk11Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
		.shaderDrawParameters = true
	};

	VkPhysicalDeviceVulkan12Features enabledVk12Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &enabledVk11Features,
		.descriptorIndexing = true, .shaderSampledImageArrayNonUniformIndexing = true,
		.descriptorBindingVariableDescriptorCount = true,
		.runtimeDescriptorArray = true,
		.bufferDeviceAddress = true };

	VkPhysicalDeviceVulkan13Features enabledVk13Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &enabledVk12Features,
		.synchronization2 = true,
		.dynamicRendering = true };

	VkPhysicalDeviceFeatures enabledVk10Features{ .samplerAnisotropy = VK_TRUE };
	const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDeviceCreateInfo deviceCI{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &enabledVk13Features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCI,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &enabledVk10Features
	};
	chk(vkCreateDevice(aCore.devices[aCore.deviceIndex], &deviceCI, nullptr, &aCore.device));
	volkLoadDevice(aCore.device);

	vkGetDeviceQueue(aCore.device, queueFamily, 0, &aCore.queue);
}

void VulkanApplication::initVulkanCore(int argc, char* argv[]) {
	volkInitialize();

	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "How to Vulkan",
		.apiVersion = VK_API_VERSION_1_3 };

	uint32_t instanceExtensionsCount{ 0 };
	char const* const* instanceExtensions{ SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount) };
	VkInstanceCreateInfo instanceCI{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = instanceExtensionsCount,
		.ppEnabledExtensionNames = instanceExtensions,
	};

	chk(vkCreateInstance(&instanceCI, nullptr, &aCore.instance));
	volkLoadInstance(aCore.instance); // ADDED: Load instance functions

	uint32_t deviceCount{ 0 };
	chk(vkEnumeratePhysicalDevices(aCore.instance, &deviceCount, nullptr));
	aCore.devices.resize(deviceCount);
	chk(vkEnumeratePhysicalDevices(aCore.instance, &deviceCount, aCore.devices.data()));

	if (argc > 1) {
		aCore.deviceIndex = std::stoi(argv[1]);
		assert(aCore.deviceIndex < deviceCount);
	}
	VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
	vkGetPhysicalDeviceProperties2(aCore.devices[aCore.deviceIndex], &deviceProperties);
	std::cout << "Selected device: " << deviceProperties.properties.deviceName << "\n";
}

void VulkanApplication::initVulkan(int argc, char* argv[]) {
	initVulkanCore(argc, argv);
	createDevice();
	createVulkanMemoryAllocator();
	swapChain.createSwapchain(aCore);

	// Depth attachment
	std::vector<VkFormat> depthFormatList{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
	for (VkFormat& format : depthFormatList) {
		VkFormatProperties2 formatProperties{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
		vkGetPhysicalDeviceFormatProperties2(aCore.devices[aCore.deviceIndex], format, &formatProperties);
		if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			depthFormat = format;
			break;
		}
	}
	assert(depthFormat != VK_FORMAT_UNDEFINED);
	VkImageCreateInfo depthImageCI{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = depthFormat,
		.extent{.width = static_cast<uint32_t>(aCore.windowSize.x), .height = static_cast<uint32_t>(aCore.windowSize.y), .depth = 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	VmaAllocationCreateInfo allocCI{ .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
	chk(vmaCreateImage(aCore.allocator, &depthImageCI, &allocCI, &depthImage, &depthImageAllocation, nullptr));
	VkImageViewCreateInfo depthViewCI{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = depthImage, .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = depthFormat, .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 } };
	chk(vkCreateImageView(aCore.device, &depthViewCI, nullptr, &depthImageView));

	pipeline.createPipeline(aCore);
}