#define VMA_IMPLEMENTATION
#define VOLK_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include "include/vulkanRenderer.h"

void VulkanApplication::run(int argc, char* argv[]) {
	initWindow();
	initVulkan(argc, argv);
	createCommandsAndSync();
}

void VulkanApplication::initWindow() {
	chk(SDL_Init(SDL_INIT_VIDEO) == true);
	aCore.window = SDL_CreateWindow("How to Vulkan", 1280u, 720u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	assert(aCore.window);
	chk(SDL_GetWindowSize(aCore.window, &aCore.windowSize.x, &aCore.windowSize.y));
}

void VulkanApplication::createPipeline() {

	std::string shaderCode = shaderProcessor.readUpShader("C:\\Users\\fox1942\\Documents\\Projects\\HowToVulkan\\source\\bin\\Debug\\triangle.slang");

	VkShaderModule vertShaderModule = shaderProcessor.compileSlangToShaderModule(aCore.device, shaderCode.c_str(), "vsMain");
	VkShaderModule fragShaderModule = shaderProcessor.compileSlangToShaderModule(aCore.device, shaderCode.c_str(), "psMain");

	VkPipelineLayoutCreateInfo layoutInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	chk(vkCreatePipelineLayout(aCore.device, &layoutInfo, nullptr, &pipelineLayout));

	VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
	VkPipelineRenderingCreateInfo pipelineRenderingInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &colorFormat
	};

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vertShaderModule, .pName = "main"},
		{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fragShaderModule, .pName = "main"}
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkPipelineViewportStateCreateInfo viewportState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};

	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f
	};

	VkPipelineMultisampleStateCreateInfo multisampling{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo colorBlending{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState{
		.sType =VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates
	};

	VkGraphicsPipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipelineRenderingInfo,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState =&inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = pipelineLayout
	};

	chk(vkCreateGraphicsPipelines(aCore.device, VK_NULL_HANDLE,1,&pipelineInfo, nullptr, &pipeline));
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


	createPipeline();
}

void VulkanApplication::createCommandsAndSync() {

	VkCommandPoolCreateInfo commandPoolCI{ .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, .queueFamilyIndex = 0 };
	chk(vkCreateCommandPool(aCore.device, &commandPoolCI, nullptr, &commandPool));

	VkCommandBufferAllocateInfo cbAllocCI{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = commandPool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = maxFramesInFlight };
	chk(vkAllocateCommandBuffers(aCore.device, &cbAllocCI, commandBuffers.data()));

	VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	for (auto i = 0; i < maxFramesInFlight; i++) {
		chk(vkCreateSemaphore(aCore.device, &semaphoreCI, nullptr, &imageAvailableSemaphores[i]));
	}
	renderCompleteSemaphores.resize(swapChain.swapchainImages.size());
	for (auto& semaphore : renderCompleteSemaphores) {
		chk(vkCreateSemaphore(aCore.device, &semaphoreCI, nullptr, &semaphore));
	}

	VkFenceCreateInfo fenceCI{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (auto i = 0; i < maxFramesInFlight; i++) {
		chk(vkCreateFence(aCore.device, &fenceCI, nullptr, &fences[i]));
	}
}

void VulkanApplication::drawFrame() {
	// 1. Várunk, hogy a GPU végezzen ennek a slotnak az előző munkájával
	chk(vkWaitForFences(aCore.device, 1, &fences[aCore.frameIndex], VK_TRUE, UINT64_MAX));

	// 2. Kép kérése (a fence-t csak utána reseteljük!)
	chk(vkAcquireNextImageKHR(aCore.device, swapChain.swapchain, UINT64_MAX,
		imageAvailableSemaphores[aCore.frameIndex], VK_NULL_HANDLE, &aCore.imageIndex));

	chk(vkResetFences(aCore.device, 1, &fences[aCore.frameIndex]));

	// 3. Command buffer felvétele
	auto cb = commandBuffers[aCore.frameIndex];
	chk(vkResetCommandBuffer(cb, 0));

	VkCommandBufferBeginInfo cbBI{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	chk(vkBeginCommandBuffer(cb, &cbBI));

	// 4. Barrier rajzolás ELŐTT: UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL
	VkImageMemoryBarrier2 toAttachment{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // egyezik a wait semaphore stage-ével
		.srcAccessMask = VK_ACCESS_2_NONE,
		.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.image = swapChain.swapchainImages[aCore.imageIndex],
		.subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
	};
	VkDependencyInfo depToAttachment{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &toAttachment,
	};
	vkCmdPipelineBarrier2(cb, &depToAttachment);

	// 5. Rajzolás
	VkRenderingAttachmentInfo colorAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = swapChain.swapchainImageViews[aCore.imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = {.color = {.float32 = { 0.1f, 0.7f, 0.7f, 1.0f } } },
	};

	VkRenderingInfo renderingInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea{.extent = swapChain.swapchainExtent },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentInfo,
	};

	vkCmdBeginRendering(cb, &renderingInfo);

	vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport{
		.x =0.0f, .y =0.0f,
		.width = static_cast<float>(swapChain.swapchainExtent.width),
		.height = static_cast<float>(swapChain.swapchainExtent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	vkCmdSetViewport(cb,0,1,&viewport);

	VkRect2D scissor{ .offset = {0,0}, .extent = swapChain.swapchainExtent };
	vkCmdSetScissor(cb, 0, 1, &scissor);

	vkCmdDraw(cb, 3, 1, 0, 0);

	vkCmdEndRendering(cb);

	// 6. Barrier rajzolás UTÁN: COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC_KHR
	VkImageMemoryBarrier2 toPresent{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_NONE,   // a presentet a szemafor sorrendezi
		.dstAccessMask = VK_ACCESS_2_NONE,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.image = swapChain.swapchainImages[aCore.imageIndex],
		.subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
	};
	VkDependencyInfo depToPresent{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &toPresent,
	};
	vkCmdPipelineBarrier2(cb, &depToPresent);

	chk(vkEndCommandBuffer(cb));

	// 7. Beküldés
	VkSemaphoreSubmitInfo waitSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = imageAvailableSemaphores[aCore.frameIndex],
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
	};

	VkCommandBufferSubmitInfo commandBufferSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = cb,
	};

	VkSemaphoreSubmitInfo signalSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = renderCompleteSemaphores[aCore.imageIndex],
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,  // minden kész legyen, a layout váltás is
	};

	VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &waitSemaphoreInfo,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &commandBufferSubmitInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo,
	};
	chk(vkQueueSubmit2(aCore.queue, 1, &submitInfo, fences[aCore.frameIndex]));

	// 8. Megjelenítés
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderCompleteSemaphores[aCore.imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &swapChain.swapchain,
		.pImageIndices = &aCore.imageIndex,
	};
	chk(vkQueuePresentKHR(aCore.queue, &presentInfo));

	aCore.frameIndex = (aCore.frameIndex + 1) % maxFramesInFlight;
}