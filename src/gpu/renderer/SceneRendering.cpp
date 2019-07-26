#include "SceneRendering.h"

#include <fstream>

#include "VulkanRenderer.h"

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

void SceneRendering::init(VulkanRenderer * renderer)
{
	this->renderer = renderer;
	this->device = renderer->device;
}

vk::ShaderModule SceneRendering::createShaderModule(const std::vector<char>& code)
{
	vk::ShaderModuleCreateInfo createInfo{ vk::ShaderModuleCreateFlags(),
		code.size(),
		reinterpret_cast<const uint32_t*>(code.data()) };
	return device.createShaderModule(createInfo);
}

void SceneRendering::createRenderPass()
{
	vk::AttachmentDescription colorAttachment{};
	colorAttachment
		.setFormat(surfaceFormat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eTransferSrcOptimal);

	vk::AttachmentReference colorAttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal };
	vk::SubpassDescription subpass{};
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentReference);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead
			| vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(1)
		.setPAttachments(&colorAttachment)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	renderPass = device.createRenderPass(renderPassInfo);
}

void SceneRendering::createGraphicsPipeline()
{
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	auto vertShaderModule = createShaderModule(vertShaderCode);
	auto fragShaderModule = createShaderModule(fragShaderCode);

	vk::PipelineShaderStageCreateInfo vertStageInfo{
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eVertex,
		vertShaderModule,
		"main"
	};

	vk::PipelineShaderStageCreateInfo fragStageInfo{
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eFragment,
		fragShaderModule,
		"main"
	};

	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };

	auto bindingDescription = VulkanVertex::getBindingDescription();
	auto attrDescriptions = VulkanVertex::getAttributeDescriptions();

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&bindingDescription)
		.setVertexAttributeDescriptionCount((uint32_t)attrDescriptions.size())
		.setPVertexAttributeDescriptions(attrDescriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly
		.setTopology(vk::PrimitiveTopology::eTriangleList)
		.setPrimitiveRestartEnable(VK_FALSE);

	vk::Viewport viewport{
		0.0f, 0.0f,
		(float)renderer->swapChainExtent.width,(float)renderer->swapChainExtent.height,
		0.0f, 1.0f // min/max depth
	};
	vk::Rect2D scissor{ {0,0}, renderer->swapChainExtent };

	vk::PipelineViewportStateCreateInfo viewportState{
		vk::PipelineViewportStateCreateFlags(),
		1, &viewport,
		1, &scissor
	};

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.0f)
		.setCullMode(vk::CullModeFlagBits::eNone)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthBiasEnable(VK_FALSE);

	vk::PipelineMultisampleStateCreateInfo multisampling{};
	multisampling
		.setSampleShadingEnable(VK_FALSE)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR
			| vk::ColorComponentFlagBits::eG
			| vk::ColorComponentFlagBits::eB
			| vk::ColorComponentFlagBits::eA)
		.setBlendEnable(VK_FALSE);

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(&colorBlendAttachment);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(1)
		.setPSetLayouts(&descriptorSetLayout);
	pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStageCount(2)
		.setPStages(shaderStages)
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPColorBlendState(&colorBlending)
		.setLayout(pipelineLayout)
		.setRenderPass(renderPass)
		.setSubpass(0);

	graphicsPipeline = device.createGraphicsPipeline(nullptr, pipelineInfo);


	device.destroyShaderModule(vertShaderModule);
	device.destroyShaderModule(fragShaderModule);
}

void SceneRendering::createCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo
		.setQueueFamilyIndex(renderer->graphicsFamilyIndice)
		.setFlags(
			vk::CommandPoolCreateFlagBits::eTransient
			| vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	commandPool = device.createCommandPool(poolInfo);

	vk::CommandBufferAllocateInfo allocInfo{
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		2
	};

	auto cmds = device.allocateCommandBuffers(allocInfo);
	renderCmd = cmds[0];
	copyStagingCmd = cmds[1];
}

void SceneRendering::createRenderImage()
{
	// we will read from this image in the shader and copy the renderimage to it
	renderer->createImage(readImage, readImageMemory,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		surfaceFormat);
	// transition the image from undefined to shaderreadonly
	renderer->startupImageTransition(readImage, surfaceFormat,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);


	renderer->createImage(renderImage, renderImageMemory,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment,
		surfaceFormat);
	// transition the image from undefined to transferdst
	renderer->startupImageTransition(renderImage, surfaceFormat,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal);

	// create the image view and framebuffer for the renderImage and readImage
	vk::ImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo
		.setImage(renderImage)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(surfaceFormat);
	imageViewCreateInfo.subresourceRange
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);
	renderImageView = device.createImageView(imageViewCreateInfo);

	// everything else is the same
	imageViewCreateInfo.setImage(readImage);
	readImageView = device.createImageView(imageViewCreateInfo);

	vk::FramebufferCreateInfo framebufferInfo{};
	framebufferInfo
		.setRenderPass(renderPass)
		.setAttachmentCount(1)
		.setPAttachments(&renderImageView)
		.setWidth(renderer->swapChainExtent.width)
		.setHeight(renderer->swapChainExtent.height)
		.setLayers(1);

	renderFramebuffer = device.createFramebuffer(framebufferInfo);

	// create the sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		// no bilinear filtering
		.setMagFilter(vk::Filter::eNearest)
		.setMinFilter(vk::Filter::eNearest)
		// may need to change that
		.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
		.setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
		.setAddressModeW(vk::SamplerAddressMode::eClampToBorder)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		// no anisotropy
		.setAnisotropyEnable(VK_FALSE)
		// coordinates are the PSX framebuffer coordinates
		.setUnnormalizedCoordinates(VK_TRUE)
		.setCompareEnable(VK_FALSE);
	readImageSampler = device.createSampler(samplerInfo);
}

void SceneRendering::createSyncObjects()
{
	renderFence = device.createFence({ vk::FenceCreateFlagBits::eSignaled });
	verticesRenderedSemaphore = device.createSemaphore({});
	copyToReadImgReady = device.createSemaphore({});
}

void SceneRendering::createBuffers()
{
	vk::DeviceSize vertexBufferSize = sizeof(VulkanVertex)*maxVerticesPerFrame;
	renderer->createBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible,
		vertexBuffer, vertexBufferMemory);

	vk::DeviceSize stagingBufferSize = sizeof(u16) * 512 * 1024;
	renderer->createBuffer(stagingBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		stagingBuffer, stagingBufferMemory);
}

void SceneRendering::createDescriptors()
{
	vk::DescriptorSetLayoutBinding imgLayoutBinding{};
	imgLayoutBinding
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment)
		.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo
		.setBindingCount(1)
		.setPBindings(&imgLayoutBinding);
	descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

	vk::DescriptorPoolSize poolSize{};
	poolSize
		.setType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1);
	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo
		.setPoolSizeCount(1)
		.setPPoolSizes(&poolSize)
		.setMaxSets(1);
	descriptorPool = device.createDescriptorPool(poolInfo);

	vk::DescriptorSetAllocateInfo allocInfo{};
	allocInfo
		.setDescriptorPool(descriptorPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&descriptorSetLayout);
	descriptorSet = device.allocateDescriptorSets(allocInfo)[0];

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(readImageView)
		.setSampler(readImageSampler);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDescriptorCount(1)
		.setDstSet(descriptorSet)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImageInfo(&imageInfo);
	device.updateDescriptorSets(descriptorWrite, nullptr);

}

void SceneRendering::createCopyCmdBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo
		.setCommandPool(renderer->copyCommandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	copyRenderToReadCmdBuffer = device.allocateCommandBuffers(allocInfo)[0];

	vk::CommandBufferBeginInfo beginInfo{};
	copyRenderToReadCmdBuffer.begin(beginInfo);
	// set the image from sampled to transferdst
	// previous content is now useless
	renderer->makeTransitionImageLayoutCmd(readImage, surfaceFormat,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
		copyRenderToReadCmdBuffer, vk::CommandBufferUsageFlags(), false);

	// copy the render image to the readImage
	vk::ImageCopy imageCopy{};
	imageCopy
		.setSrcOffset({ 0,0,0 })
		.setDstOffset({ 0,0,0 })
		.setExtent({ renderer->swapChainExtent.width, renderer->swapChainExtent.height, 1 });
	imageCopy.srcSubresource
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setMipLevel(0)
		.setBaseArrayLayer(0)
		.setLayerCount(1);
	imageCopy.dstSubresource
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setMipLevel(0)
		.setBaseArrayLayer(0)
		.setLayerCount(1);
	copyRenderToReadCmdBuffer.copyImage(renderImage, vk::ImageLayout::eTransferSrcOptimal,
		readImage, vk::ImageLayout::eTransferDstOptimal, imageCopy);

	// switch back the image to shader_read
	renderer->makeTransitionImageLayoutCmd(readImage, surfaceFormat,
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		copyRenderToReadCmdBuffer, vk::CommandBufferUsageFlags(), false);
	copyRenderToReadCmdBuffer.end();
}

void SceneRendering::makeRenderCmdBuffer()
{
	renderCmd.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	vk::ClearValue clearColor{};
	// unused
	clearColor.color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(renderPass)
		.setFramebuffer(renderFramebuffer)
		.setRenderArea({ {0,0}, renderer->swapChainExtent })
		.setClearValueCount(1)
		.setPClearValues(&clearColor);

	renderCmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	renderCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
	renderCmd.bindVertexBuffers(0, vertexBuffer, { 0 });
	renderCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		0, descriptorSet, nullptr);

	renderCmd.draw((uint32_t)verticesToRenderSize, 1, 0, 0);

	renderCmd.endRenderPass();
	renderCmd.end();
}

void SceneRendering::copyVertices() {
	assert(verticesToRenderSize <= maxVerticesPerFrame);
	size_t blockSize = renderer->physicalDeviceProperties.limits.nonCoherentAtomSize;
	size_t copySize = ((sizeof(VulkanVertex)*verticesToRenderSize / blockSize) + 1) * blockSize;

	vk::MappedMemoryRange vertexMemoryRange{};
	vertexMemoryRange
		.setMemory(vertexBufferMemory)
		.setOffset(0)
		// a size containing the currently rendered vertices is enough
		.setSize(blockSize);

	void* data = device.mapMemory(vertexBufferMemory, 0, blockSize);
	memcpy(data, verticesToRender, sizeof(VulkanVertex)*verticesToRenderSize);


	device.flushMappedMemoryRanges(vertexMemoryRange);
	device.unmapMemory(vertexBufferMemory);
}

void SceneRendering::renderVertices()
{
	device.waitForFences(renderFence, VK_TRUE,
		std::numeric_limits<uint64_t>::max());
	device.resetFences(renderFence);


	copyVertices();
	makeRenderCmdBuffer();

	vk::Semaphore renderFinishedSemaphores[]
		= {verticesRenderedSemaphore, copyToReadImgReady};
	vk::SubmitInfo renderSubmitInfo{};
	renderSubmitInfo
		// no semaphores to wait for
		.setWaitSemaphoreCount(0)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&renderCmd)
		.setSignalSemaphoreCount(2)
		.setPSignalSemaphores(renderFinishedSemaphores);

	renderer->graphicsQueue.submit(renderSubmitInfo, nullptr);

	vk::SubmitInfo copySubmitInfo{};
	// not sure about this one
	vk::PipelineStageFlags copyStage = vk::PipelineStageFlagBits::eTransfer;
	copySubmitInfo
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&copyToReadImgReady)
		.setPWaitDstStageMask(&copyStage)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&copyRenderToReadCmdBuffer)
		.setSignalSemaphoreCount(0);

	renderer->graphicsQueue.submit(copySubmitInfo, renderFence);

}

void SceneRendering::transferImage(u16* image, Point<i16> topLeft, Point<i16> extent)
{
	assert(topLeft.x >= 0 && topLeft.y >= 0 && extent.x > 0 && extent.y > 0);
	u32 size = (u32)(extent.x * extent.y);

	// change the data from ABGR to ARGB
	for (int i = 0; i < size; i++) {
		u16 pixel = image[i];
		image[i] = (pixel & 0b1000001111100000) // alpha and green
			| ((pixel & 0b11111) << 10) // red
			| (pixel >> 10) & 0b11111; // blue
	}

	// copy the data
	void* data = device.mapMemory(stagingBufferMemory, 0, VK_WHOLE_SIZE);
	memcpy(data, image, sizeof(u16) * size);
	device.unmapMemory(stagingBufferMemory);

	// wait for the previous operations to end
	device.waitForFences(renderFence, VK_TRUE,
		std::numeric_limits<uint64_t>::max());
	device.resetFences(renderFence);

	// make the new command buffer
	copyStagingCmd.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	// first transition
	renderer->makeTransitionImageLayoutCmd(readImage, surfaceFormat,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal,
		copyStagingCmd, vk::CommandBufferUsageFlags(), false);

	// copy the buffer to the readImage
	vk::BufferImageCopy bufferImgCopy{};
	bufferImgCopy
		.setBufferImageHeight(0) // tightly packed data
		.setBufferRowLength(0)
		.setBufferOffset(0)
		.setImageExtent({ (u32)extent.x, (u32)extent.y, 1 })
		.setImageOffset({ (u32)topLeft.x, (u32)topLeft.y, 0 });
	bufferImgCopy.imageSubresource
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setMipLevel(0)
		.setBaseArrayLayer(0)
		.setLayerCount(1);
	copyStagingCmd.copyBufferToImage(stagingBuffer, readImage,
		vk::ImageLayout::eTransferDstOptimal, bufferImgCopy);

	// back to shader_read_only
	renderer->makeTransitionImageLayoutCmd(readImage, surfaceFormat,
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		copyStagingCmd, vk::CommandBufferUsageFlags(), false);

	copyStagingCmd.end();

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphoreCount(0)
		.setSignalSemaphoreCount(0)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&copyStagingCmd);
	renderer->graphicsQueue.submit(submitInfo,renderFence);
}

void SceneRendering::destroy()
{
	device.destroyFence(renderFence);
	device.destroySemaphore(verticesRenderedSemaphore);
	device.destroySemaphore(copyToReadImgReady);

	device.destroyDescriptorPool(descriptorPool);
	device.destroyDescriptorSetLayout(descriptorSetLayout);

	device.destroySampler(readImageSampler);
	device.destroyImageView(readImageView);
	device.destroyImage(readImage);
	device.freeMemory(readImageMemory);
	device.destroyFramebuffer(renderFramebuffer);
	device.destroyImageView(renderImageView);
	device.destroyImage(renderImage);
	device.freeMemory(renderImageMemory);

	device.destroyBuffer(stagingBuffer);
	device.freeMemory(stagingBufferMemory);
	device.destroyBuffer(vertexBuffer);
	device.freeMemory(vertexBufferMemory);

	device.destroyCommandPool(commandPool);

	device.destroyPipeline(graphicsPipeline);
	device.destroyPipelineLayout(pipelineLayout);
	device.destroyRenderPass(renderPass);
}