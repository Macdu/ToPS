#pragma once

#include <vulkan/vulkan.hpp>

#include "../../definitions.h"
#include "VulkanVertex.h"

class VulkanRenderer;

// A class containing everything related to rendering the PS framebuffer to renderImage
class SceneRendering {
public:
	static const vk::Format surfaceFormat = vk::Format::eA1R5G5B5UnormPack16;

	VulkanRenderer* renderer;
	vk::Device device;

	vk::RenderPass renderPass;
	vk::PipelineLayout pipelineLayout;
	vk::Pipeline graphicsPipeline;

	vk::Buffer vertexBuffer;
	vk::DeviceMemory vertexBufferMemory;

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	vk::CommandPool commandPool;
	vk::CommandBuffer renderCmd;
	vk::CommandBuffer copyStagingCmd;

	// The image containing the current PSX framebuffer
	vk::Image readImage;
	vk::DeviceMemory readImageMemory;
	vk::ImageView readImageView;
	vk::Sampler readImageSampler;
	// Image where the PSX framebuffer is drawn to
	vk::Image renderImage;
	vk::DeviceMemory renderImageMemory;
	vk::ImageView renderImageView;
	vk::Framebuffer renderFramebuffer;

	vk::DescriptorSetLayout descriptorSetLayout;
	vk::DescriptorPool descriptorPool;
	vk::DescriptorSet descriptorSet;

	static const int maxVerticesPerFrame = 100000;

	// store the current vertices before they are rendered
	VulkanVertex verticesToRender[maxVerticesPerFrame];
	int verticesToRenderSize = 0;

	vk::Fence renderFence;
	vk::Semaphore verticesRenderedSemaphore;
	vk::Semaphore copyToReadImgReady;
	vk::CommandBuffer copyRenderToReadCmdBuffer;

	void init(VulkanRenderer* renderer);

	vk::ShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();
	void createGraphicsPipeline();
	void createCommandPool();
	void createRenderImage();
	void createSyncObjects();
	void createBuffers();
	void createCopyCmdBuffer();
	void createDescriptors();

	void makeRenderCmdBuffer();
	void copyVertices();
	void renderVertices();

	// transfer an image from DMA to the framebuffer
	void transferImage(u16* image, Point<i16> topLeft, Point<i16> extent);

	void destroy();

};