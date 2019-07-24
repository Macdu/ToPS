#pragma once

#include "SceneRendering.h"

class VulkanRenderer {
private:
	const std::array<const char*, 1> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
public:
	SceneRendering sceneRendering;

	vk::Instance instance;
	uint32_t width, height;

	vk::PhysicalDevice physicalDevice;
	vk::PhysicalDeviceProperties physicalDeviceProperties;
	int graphicsFamilyIndice = -1;
	int presentFamilyIndice = -1;

	vk::Device device;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	vk::SurfaceKHR surface;

	vk::SwapchainKHR swapChain;
	std::vector<vk::Image> swapChainImages;
	vk::Format swapChainImageFormat;
	vk::Extent2D swapChainExtent;

	vk::Semaphore imageAvailableSemaphore;
	vk::Semaphore setTransferDoneSemaphore;
	vk::Semaphore copyFinishedSemaphore;
	vk::Semaphore setPresentDoneSemaphore;
	vk::Fence inFlightFence;


	// command pool and buffers related to copying the previous images
	vk::CommandPool copyCommandPool;
	std::vector<vk::CommandBuffer> swapChainCopyCmds;
	std::vector<vk::CommandBuffer> swapChainSetTransferCmds;
	std::vector<vk::CommandBuffer> swapChainSetPresentCmds;


public:
	void pickPhysicalDevice();
	vk::PhysicalDevice getSuitableDevice();
	void findQueues();
	void createLogicalDevice();
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createCommandPool();
	void makeCopyCommandBuffers();
	
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	void makeTransitionImageLayoutCmd(vk::Image image, vk::Format format,
		vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
		vk::CommandBuffer &commandBuffer, 
		vk::CommandBufferUsageFlags cmdUsage = vk::CommandBufferUsageFlags(),
		bool cmdBeginEnd = true);
	void startupImageTransition(vk::Image image, vk::Format format,
		vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlagBits usage,
		vk::MemoryPropertyFlagBits properties, vk::Buffer& buffer, vk::DeviceMemory &bufferMemory);
	void createImage(vk::Image &image, vk::DeviceMemory &memory, vk::ImageUsageFlags usage);

	void createSyncObjects();

	void initVulkan();
	void drawFrame();
	void cleanup();

	void setWidth(int width) {
		this->width = width;
	}
	void setHeight(int height) {
		this->height = height;
	}

	vk::Instance getInstance() {
		return instance;
	}

	void setInstance(vk::Instance instance) {
		this->instance = instance;
	}

	void setSurface(vk::SurfaceKHR surface) {
		this->surface = surface;
	}

};