#include "VulkanRenderer.h"

#include <set>

#include <QDebug>

#include "../GPUProperties.h"

void VulkanRenderer::pickPhysicalDevice()
{
	physicalDevice = getSuitableDevice();
	physicalDeviceProperties = physicalDevice.getProperties();
}

vk::PhysicalDevice VulkanRenderer::getSuitableDevice()
{
	for (auto& device : instance.enumeratePhysicalDevices()) {
		if (device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu
			&& device.getFeatures().geometryShader) {
			auto props = device.enumerateDeviceExtensionProperties();
			for (auto& prop : device.enumerateDeviceExtensionProperties()) {
				if (std::strcmp(prop.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
					return device;
				}
			}
		}
	}
	qFatal("No physical device found!");
}

void VulkanRenderer::findQueues()
{
	int indice = 0;
	for (const auto& queueFamily : physicalDevice.getQueueFamilyProperties()) {
		if (graphicsFamilyIndice == -1
			&& queueFamily.queueCount > 0
			&& (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
			graphicsFamilyIndice = indice;
		}

		if (presentFamilyIndice == -1
			&& queueFamily.queueCount > 0
			&& physicalDevice.getSurfaceSupportKHR(indice, surface)) {
			presentFamilyIndice = indice;
		}

		indice++;
	}
	if (graphicsFamilyIndice == -1 || presentFamilyIndice == -1) {
		qFatal("No graphics queue found !");
	}
}

void VulkanRenderer::createLogicalDevice()
{
	float queuePriority = 1.0f;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

	std::set<uint32_t> queueFamilies = {
		(uint32_t)graphicsFamilyIndice,
		(uint32_t)presentFamilyIndice
	};
	for (uint32_t queueFamily : queueFamilies) {
		queueCreateInfos.emplace_back(
			vk::DeviceQueueCreateFlags(),
			queueFamily, 1,
			&queuePriority);
	}

	vk::PhysicalDeviceFeatures deviceFeatures{};
	vk::DeviceCreateInfo createInfo{};
	createInfo
		.setPQueueCreateInfos(queueCreateInfos.data())
		.setQueueCreateInfoCount((uint32_t)queueCreateInfos.size())
		.setPEnabledFeatures(&deviceFeatures)
		.setEnabledExtensionCount((uint32_t)deviceExtensions.size())
		.setPpEnabledExtensionNames(deviceExtensions.data());

	device = physicalDevice.createDevice(createInfo);
	graphicsQueue = device.getQueue(graphicsFamilyIndice, 0);
	presentQueue = device.getQueue(presentFamilyIndice, 0);
}

vk::SurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	vk::SurfaceFormatKHR wanted = { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
	if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
		return wanted;
	}

	if (std::find(availableFormats.begin(), availableFormats.end(), wanted) != availableFormats.end())
		return wanted;

	return availableFormats[0];
}

vk::PresentModeKHR VulkanRenderer::choosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
{
	if (std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eMailbox) != presentModes.end())
		return vk::PresentModeKHR::eMailbox;

	if (std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eImmediate) != presentModes.end())
		return vk::PresentModeKHR::eImmediate;

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR & capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	vk::Extent2D actualExtent = { width, height };
	actualExtent.width = std::max(capabilities.minImageExtent.width,
		std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(capabilities.minImageExtent.height,
		std::min(capabilities.maxImageExtent.height, actualExtent.height));
	return actualExtent;
}

void VulkanRenderer::createSwapChain()
{
	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
	vk::PresentModeKHR presentMode = choosePresentMode(physicalDevice.getSurfacePresentModesKHR(surface));
	vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	vk::Extent2D extent = chooseSwapExtent(capabilities);
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0) {
		imageCount = std::min(imageCount, capabilities.maxImageCount);
	}

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo
		.setSurface(surface)
		.setMinImageCount(imageCount)
		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(/*vk::ImageUsageFlagBits::eColorAttachment | */vk::ImageUsageFlagBits::eTransferDst)
		.setPreTransform(capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(VK_TRUE);

	uint32_t queueIndices[] = { (uint32_t)graphicsFamilyIndice,(uint32_t)presentFamilyIndice };
	if (queueIndices[0] == queueIndices[1]) {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}
	else {
		createInfo
			.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(2)
			.setPQueueFamilyIndices(queueIndices);
	}

	swapChain = device.createSwapchainKHR(createInfo);
	swapChainImages = device.getSwapchainImagesKHR(swapChain);

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void VulkanRenderer::createCommandPool()
{
	copyCommandPool = device.createCommandPool({});
}

void VulkanRenderer::makeCopyCommandBuffers()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo
		.setCommandPool(copyCommandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount((uint32_t)swapChainImages.size());
	swapChainCopyCmds = device.allocateCommandBuffers(allocInfo);
	swapChainSetTransferCmds = device.allocateCommandBuffers(allocInfo);
	swapChainSetPresentCmds = device.allocateCommandBuffers(allocInfo);

	// make command buffers related to rendered framebuffer to swapchain copying
	vk::ImageBlit imageBlit{};
	std::array<vk::Offset3D, 2> blitOffsets;
	blitOffsets[0] = { 0,0,0 };
	blitOffsets[1] = { (i32)swapChainExtent.width, (i32)swapChainExtent.height, 1 };
	imageBlit
		.setSrcOffsets(blitOffsets)
		.setDstOffsets(blitOffsets);
	imageBlit.srcSubresource
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setMipLevel(0)
		.setBaseArrayLayer(0)
		.setLayerCount(1);
	imageBlit.dstSubresource
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setMipLevel(0)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	for (int i = 0; i < swapChainImages.size(); i++) {
		vk::CommandBuffer swapChainCopyCmd = swapChainCopyCmds[i];
		vk::CommandBufferBeginInfo cmdBeginInfo{};
		swapChainCopyCmd.begin(cmdBeginInfo);
		swapChainCopyCmd.blitImage(sceneRendering.renderImage, vk::ImageLayout::eTransferSrcOptimal,
			swapChainImages[i], vk::ImageLayout::eTransferDstOptimal,
			imageBlit, vk::Filter::eNearest);
		swapChainCopyCmd.end();
	}

	// make the command buffer related to swapchain transitions : from UNDEFINED to TRANSFER_DST
	// and from TRANSFER_DST to PRESENT_KHR
	for (int i = 0; i < swapChainImages.size(); i++) {
		vk::CommandBuffer swapChainSetTransferCmd = swapChainSetTransferCmds[i];

		makeTransitionImageLayoutCmd(swapChainImages[i], swapChainImageFormat,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
			swapChainSetTransferCmds[i]);

		makeTransitionImageLayoutCmd(swapChainImages[i], swapChainImageFormat,
			vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
			swapChainSetPresentCmds[i]);
	}
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	auto memProperties = physicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i))
			&& ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
			return i;
		}
	}
	qFatal("Failed to find a suitable memory type!");
}

void VulkanRenderer::makeTransitionImageLayoutCmd(vk::Image image, vk::Format format,
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	vk::CommandBuffer &commandBuffer,
	vk::CommandBufferUsageFlags cmdUsage,
	bool cmdBeginEnd)
{
	if (cmdBeginEnd) {
		commandBuffer.begin({ cmdUsage });
	}

	vk::ImageMemoryBarrier barrier{};
	barrier
		.setOldLayout(oldLayout)
		.setNewLayout(newLayout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(image);
	barrier.subresourceRange
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	vk::PipelineStageFlags sourceStage, destinationStage;
	if (oldLayout == vk::ImageLayout::eUndefined 
		&& newLayout == vk::ImageLayout::eTransferSrcOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlags());
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined
		&& newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlags());
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined
		&& newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlags());
		barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal
		&& newLayout == vk::ImageLayout::ePresentSrcKHR) {
		barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barrier.setDstAccessMask(vk::AccessFlags());

		// presentation is not part of the pipeline
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal
		&& newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal
		&& newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlags());
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}

	commandBuffer.pipelineBarrier(sourceStage, destinationStage,vk::DependencyFlags(), nullptr, nullptr, barrier);
	if (cmdBeginEnd) {
		commandBuffer.end();
	}
}

void VulkanRenderer::startupImageTransition(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo
		.setCommandPool(sceneRendering.commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers(allocInfo)[0];

	makeTransitionImageLayoutCmd(image, format, oldLayout, newLayout,
		cmdBuffer, { vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setCommandBufferCount(1)
		.setPCommandBuffers(&cmdBuffer);
	graphicsQueue.submit(submitInfo, nullptr);
	graphicsQueue.waitIdle();
	device.freeCommandBuffers(sceneRendering.commandPool, cmdBuffer);
}

void VulkanRenderer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags properties, vk::Buffer & buffer,
	vk::DeviceMemory & bufferMemory)
{
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo
		.setSize(size)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);
	buffer = device.createBuffer(bufferInfo);

	auto memRequirements = device.getBufferMemoryRequirements(buffer);
	vk::MemoryAllocateInfo allocInfo{};
	allocInfo
		.setAllocationSize(memRequirements.size)
		.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits,
			properties));

	bufferMemory = device.allocateMemory(allocInfo);
	device.bindBufferMemory(buffer, bufferMemory, 0);
}

void VulkanRenderer::createSyncObjects()
{
	imageAvailableSemaphore = device.createSemaphore({});
	setTransferDoneSemaphore = device.createSemaphore({});
	copyFinishedSemaphore = device.createSemaphore({});
	setPresentDoneSemaphore = device.createSemaphore({});
	inFlightFence = device.createFence({ vk::FenceCreateFlagBits::eSignaled });
}

void VulkanRenderer::createImage(vk::Image & image, vk::DeviceMemory & memory,
	vk::ImageUsageFlags usage, vk::Format format)
{
	vk::ImageCreateInfo imageInfo{};
	imageInfo
		.setImageType(vk::ImageType::e2D)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(format)
		.setTiling(vk::ImageTiling::eOptimal)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setSamples(vk::SampleCountFlagBits::e1);
	imageInfo.extent
		.setWidth(swapChainExtent.width)
		.setHeight(swapChainExtent.height)
		.setDepth(1);

	image = device.createImage(imageInfo);
	
	vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);
	vk::MemoryAllocateInfo allocInfo{};
	allocInfo
		.setAllocationSize(memRequirements.size)
		.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eDeviceLocal));
	memory = device.allocateMemory(allocInfo);

	device.bindImageMemory(image, memory, 0);
}

void VulkanRenderer::initVulkan()
{
	pickPhysicalDevice();
	findQueues();
	createLogicalDevice();
	sceneRendering.init(this);
	createCommandPool();
	sceneRendering.createCommandPool();
	createSyncObjects();
	sceneRendering.createSyncObjects();
	createSwapChain();
	sceneRendering.createRenderPass();
	sceneRendering.createRenderImage();
	sceneRendering.createDescriptors();
	sceneRendering.createGraphicsPipeline();
	sceneRendering.createBuffers();
	makeCopyCommandBuffers();
	sceneRendering.createCopyCmdBuffer();
}

void VulkanRenderer::drawFrame()
{
	device.waitForFences(inFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	device.resetFences(inFlightFence);
	vk::ResultValue<uint32_t> acquireResult = device.acquireNextImageKHR(
		swapChain, std::numeric_limits<uint64_t>::max(),
		imageAvailableSemaphore, nullptr);

	uint32_t imageIndex = acquireResult.value;
	if (acquireResult.result != vk::Result::eSuccess) {
		qFatal("Could not acquire next Frame!");
	}

	// set the swapchain image to transfer_dst
	vk::SubmitInfo setTransferInfo{};
	vk::PipelineStageFlags setTransferFlag = vk::PipelineStageFlagBits::eTransfer;
	setTransferInfo
		.setWaitSemaphoreCount(1)
		.setPWaitDstStageMask(&setTransferFlag)
		.setPWaitSemaphores(&imageAvailableSemaphore)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&setTransferDoneSemaphore)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&swapChainSetTransferCmds[imageIndex]);
	graphicsQueue.submit(setTransferInfo, nullptr);

	// render the image
	sceneRendering.renderVertices(true);

	// copy it to the swapchain
	vk::PipelineStageFlags copyWaitStages[] = {
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer
	};
	vk::Semaphore copyWaitSemaphores[] = {
		sceneRendering.verticesRenderedSemaphore,
		setTransferDoneSemaphore
	};
	vk::SubmitInfo copySwapChainInfo{};
	copySwapChainInfo
		.setWaitSemaphoreCount(2)
		.setPWaitDstStageMask(copyWaitStages)
		.setPWaitSemaphores(copyWaitSemaphores)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&copyFinishedSemaphore)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&swapChainCopyCmds[imageIndex]);
	// and set the inflightfence to it
	graphicsQueue.submit(copySwapChainInfo, inFlightFence);

	vk::SubmitInfo setPresentInfo{};
	// not really sure about this one, maybe all commmands is better
	vk::PipelineStageFlags setPresentFlags = vk::PipelineStageFlagBits::eBottomOfPipe;
	setPresentInfo
		.setWaitSemaphoreCount(1)
		.setPWaitDstStageMask(&setPresentFlags)
		.setPWaitSemaphores(&copyFinishedSemaphore)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&setPresentDoneSemaphore)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&swapChainSetPresentCmds[imageIndex]);
	graphicsQueue.submit(setPresentInfo, nullptr);


	vk::PresentInfoKHR presentInfo{};
	presentInfo
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&setPresentDoneSemaphore)
		.setSwapchainCount(1)
		.setPSwapchains(&swapChain)
		.setPImageIndices(&imageIndex);

	presentQueue.presentKHR(presentInfo);
}

void VulkanRenderer::cleanup()
{
	device.waitIdle();

	sceneRendering.destroy();

	device.destroyFence(inFlightFence);;
	device.destroySemaphore(imageAvailableSemaphore);
	device.destroySemaphore(setTransferDoneSemaphore);
	device.destroySemaphore(copyFinishedSemaphore);
	device.destroySemaphore(setPresentDoneSemaphore);

	device.destroyCommandPool(copyCommandPool);

	device.destroySwapchainKHR(swapChain);
	device.destroy();
}
