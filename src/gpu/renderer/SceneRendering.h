#pragma once

#include <vulkan/vulkan.hpp>

#include "../../definitions.h"
#include "VulkanVertex.h"

class VulkanRenderer;

// A class containing everything related to rendering the PS framebuffer to renderImage
class SceneRendering {
public:

	VulkanRenderer* renderer;
	vk::Device device;

	vk::RenderPass renderPass;
	vk::PipelineLayout pipelineLayout;
	vk::Pipeline graphicsPipeline;

	vk::Buffer vertexBuffer;
	vk::DeviceMemory vertexBufferMemory;

	vk::CommandPool commandPool;
	vk::CommandBuffer renderCommand;

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

	const int maxVerticesPerFrame = 100000;

	// store the current vertices before they are rendered
	std::vector<VulkanVertex> verticesToRender;

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
	void createVertexBuffer();
	void createCopyCmdBuffer();
	void createDescriptors();

	void makeRenderCmdBuffer();
	void copyVertices();
	void renderVertices();

	void destroy();

};