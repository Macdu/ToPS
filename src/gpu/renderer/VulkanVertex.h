#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct VulkanVertex {
	glm::ivec2 pos;
	glm::uvec3 color;
	glm::uvec2 textCoords;
	glm::uint clutId;
	glm::uint texturePage;

	static vk::VertexInputBindingDescription getBindingDescription() {
		vk::VertexInputBindingDescription bindingDescription{};
		bindingDescription
			.setBinding(0)
			.setStride(sizeof(VulkanVertex))
			.setInputRate(vk::VertexInputRate::eVertex);
		return bindingDescription;
	}

	static std::array<vk::VertexInputAttributeDescription, 5> getAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 5> attrDescriptions{};

		attrDescriptions[0]
			.setBinding(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32Sint)
			.setOffset(offsetof(VulkanVertex, pos));
		attrDescriptions[1]
			.setBinding(0)
			.setLocation(1)
			.setFormat(vk::Format::eR32G32B32Uint)
			.setOffset(offsetof(VulkanVertex, color));
		attrDescriptions[2]
			.setBinding(0)
			.setLocation(2)
			.setFormat(vk::Format::eR32G32Uint);
		attrDescriptions[3]
			.setBinding(0)
			.setLocation(3)
			.setFormat(vk::Format::eR32Uint);
		attrDescriptions[4]
			.setBinding(0)
			.setLocation(4)
			.setFormat(vk::Format::eR32Uint);

		return attrDescriptions;
	}
};