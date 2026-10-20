#include "include/vRenderer.h"
#include "include/helper.h"
#include "include/swapChain.h"
#include "include/vPipeline.h"

void VRenderer::createCommandsAndSync(appCore& aCore, SwapChain swapChain) {

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

void VRenderer::drawFrame(appCore& aCore, SwapChain swapChain, VkPipeline pipeline) {
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
		.x = 0.0f, .y = 0.0f,
		.width = static_cast<float>(swapChain.swapchainExtent.width),
		.height = static_cast<float>(swapChain.swapchainExtent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	vkCmdSetViewport(cb, 0, 1, &viewport);

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