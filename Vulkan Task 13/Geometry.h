#pragma once

#include <array>
#include <vector>
#include <map>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shared.h"
#include "Fraction.h"
#include "CoordinateSystem.h"


namespace ak::geometry
{
	struct Uniform
	{
		alignas(16) glm::mat4 model{};
		alignas(16) glm::mat4 view{};
		alignas(16) glm::mat4 proj{};
	};


	struct Vertex2D
	{
		glm::vec2 vertex{};
		glm::vec4 color{};
	};

	struct Vertex3D
	{
		glm::vec3 vertex{};
		glm::vec4 color{};
	};


	class Primitive
	{
	public:
		virtual uint32_t getVerticesCount() const final;
		virtual std::vector<glm::vec3> getVertices() const final;
		virtual std::vector<Vertex3D> getVertices(TopologyTypeFlag topologyType) const final;
		virtual glm::vec4 getColor(TopologyTypeFlag topologyType) const final;
		virtual void setColor(TopologyTypeFlag topologyType, glm::vec4 color) final;

		virtual std::vector<uint32_t> getVertexIndices(TopologyTypeFlag topologyType) const = 0;

	protected:
		FractionVec3 center_{};
		CoordinateSystem<glm::vec3> coordinateSystem_{};			//////////// TODO Fraction
		std::vector<glm::vec3> vertices_{};
		std::array<glm::vec4, numTopologyTypes> colors_{};
	};


	// 2D

	//class Parabola2D final : public Primitive
	//{
	//public:
	//	Parabola2D() = delete;

	//	std::vector<Vertex3D> getVertices(TopologyTypeFlag topologyType) const override;
	//	std::vector<uint32_t> getVertexIndices(TopologyTypeFlag topologyType) const override;

	//private:
	//	Fraction a_{}, b_{}, c_{};
	//};

	//class Polyline2D final : public Primitive
	//{
	//public:
	//	Polyline2D() = delete;

	//	std::vector<Vertex3D> getVertices(TopologyTypeFlag topologyType) const override;
	//	std::vector<uint32_t> getVertexIndices(TopologyTypeFlag topologyType) const override;

	//private:
	//	std::vector<FractionVec2> vertices_{};
	//	Fraction level_{};
	//	FractionVec3 normal_{};
	//};

	class Circle2D final : public Primitive
	{
	public:
		Circle2D() = delete;
		Circle2D(glm::vec3 center, float radius,
			const CoordinateSystem<glm::vec3>& coordinateSystem = {},
			uint32_t numFragments = numFragments,
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		Circle2D(FractionVec3 center, Fraction radius,
			const CoordinateSystem<FractionVec3>& coordinateSystem = {},
			uint32_t numFragments = numFragments,
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		explicit Circle2D(const Circle2D& rhs);
		Circle2D& operator=(const Circle2D& rhs);
		std::vector<uint32_t> getVertexIndices(TopologyTypeFlag topologyType) const override;

	private:
		Fraction radius_{};
		uint32_t numFragments_{};
	};


	// 3D

	class Point final : public Primitive
	{
	public:
		Point() = delete;
		Point(glm::vec3 vertex,
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		Point(FractionVec3 vertex,
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		explicit Point(const Point& rhs);
		Point& operator=(const Point& rhs);
		std::vector<uint32_t> getVertexIndices(TopologyTypeFlag topologyType) const override;

	private:
		FractionVec3 pureVertex_{};
	};

	class Segment final : public Primitive
	{
	public:
		Segment() = delete;
		Segment(std::array<glm::vec3, 2> vertices,
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		Segment(std::array<FractionVec3, 2> vertices,
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		explicit Segment(const Segment& rhs);
		Segment& operator=(const Segment& rhs);
		std::vector<uint32_t> getVertexIndices(TopologyTypeFlag topologyType) const override;

	private:
		std::array<FractionVec3, 2> pureVertices_{};
	};

	class Tetrahedron final : public Primitive
	{
	public:
		Tetrahedron() = delete;
		Tetrahedron(glm::vec3 center, std::array<glm::vec3, 4> vertices,
			const CoordinateSystem<glm::vec3>& coordinateSystem = {},
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		Tetrahedron(FractionVec3 center, std::array<FractionVec3, 4> vertices,
			const CoordinateSystem<FractionVec3>& coordinateSystem = {},
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		explicit Tetrahedron(const Tetrahedron& rhs);
		Tetrahedron& operator=(const Tetrahedron& rhs);
		std::vector<uint32_t> getVertexIndices(TopologyTypeFlag topologyType) const override;

	private:
		std::array<FractionVec3, 4> pureVertices_{};
	};

	class Sphere final : public Primitive
	{
	public:
		Sphere() = delete;
		Sphere(glm::vec3 center, float radius,
			const CoordinateSystem<glm::vec3>& coordinateSystem = {},
			uint32_t numFragments = numFragments,
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		Sphere(FractionVec3 center, Fraction radius,
			const CoordinateSystem<FractionVec3>& coordinateSystem = {},
			uint32_t numFragments = numFragments,
			std::array<glm::vec4, numTopologyTypes> colors = { RGB_GRAY, RGB_BLACK, RGB_GRAY });
		explicit Sphere(const Sphere& rhs);
		Sphere& operator=(const Sphere& rhs);
		std::vector<uint32_t> getVertexIndices(TopologyTypeFlag topologyType) const override;

	private:
		Fraction radius_{};
		uint32_t numFragments_{};
		std::vector<Circle2D> circles_{};
	};


	template <typename T>
	inline vk::VertexInputBindingDescription
		getBindDescription(uint32_t binding)
	{
		return {};
	}

	template <typename T>
	inline vk::VertexInputAttributeDescription
		getAttributeDescription(uint32_t binding, uint32_t location)
	{
		return {};
	}

	template <typename T>
	inline std::array<vk::VertexInputAttributeDescription, 2>
		getAttributeDescriptions(uint32_t binding)
	{
		return {};
	}

	template <>
	inline vk::VertexInputBindingDescription
		getBindDescription<glm::vec2>(uint32_t binding)
	{
		return vk::VertexInputBindingDescription{}
			.setBinding(binding)
			.setStride(sizeof(glm::vec2))
			.setInputRate(vk::VertexInputRate::eVertex);
	}

	template <>
	inline vk::VertexInputBindingDescription
		getBindDescription<glm::vec3>(uint32_t binding)
	{
		return vk::VertexInputBindingDescription{}
			.setBinding(binding)
			.setStride(sizeof(glm::vec3))
			.setInputRate(vk::VertexInputRate::eVertex);
	}

	template <>
	inline vk::VertexInputBindingDescription
		getBindDescription<glm::vec4>(uint32_t binding)
	{
		return vk::VertexInputBindingDescription{}
			.setBinding(binding)
			.setStride(sizeof(glm::vec4))
			.setInputRate(vk::VertexInputRate::eVertex);
	}

	template <>
	inline vk::VertexInputBindingDescription
		getBindDescription<Vertex2D>(uint32_t binding)
	{
		return vk::VertexInputBindingDescription{}
			.setBinding(binding)
			.setStride(sizeof(Vertex2D))
			.setInputRate(vk::VertexInputRate::eVertex);
	}

	template <>
	inline vk::VertexInputBindingDescription
		getBindDescription<Vertex3D>(uint32_t binding)
	{
		return vk::VertexInputBindingDescription{}
			.setBinding(binding)
			.setStride(sizeof(Vertex3D))
			.setInputRate(vk::VertexInputRate::eVertex);
	}

	template <>
	inline vk::VertexInputAttributeDescription
		getAttributeDescription<glm::vec2>(uint32_t binding, uint32_t location)
	{
		return vk::VertexInputAttributeDescription{}
			.setBinding(binding)
			.setLocation(location)
			.setOffset(0)
			.setFormat(vk::Format::eR32G32Sfloat);
	}

	template <>
	inline vk::VertexInputAttributeDescription
		getAttributeDescription<glm::vec3>(uint32_t binding, uint32_t location)
	{
		return vk::VertexInputAttributeDescription{}
			.setBinding(binding)
			.setLocation(location)
			.setOffset(0)
			.setFormat(vk::Format::eR32G32B32Sfloat);
	}

	template <>
	inline vk::VertexInputAttributeDescription
		getAttributeDescription<glm::vec4>(uint32_t binding, uint32_t location)
	{
		return vk::VertexInputAttributeDescription{}
			.setBinding(binding)
			.setLocation(location)
			.setOffset(0)
			.setFormat(vk::Format::eR32G32B32A32Sfloat);
	}

	template <>
	inline std::array<vk::VertexInputAttributeDescription, 2>
		getAttributeDescriptions<Vertex2D>(uint32_t binding)
	{
		return {
			vk::VertexInputAttributeDescription{}
				.setBinding(binding)
				.setLocation(0)
				.setOffset(offsetof(Vertex2D, vertex))
				.setFormat(vk::Format::eR32G32Sfloat),
			vk::VertexInputAttributeDescription{}
				.setBinding(binding)
				.setLocation(1)
				.setOffset(offsetof(Vertex2D, color))
				.setFormat(vk::Format::eR32G32B32A32Sfloat)
		};
	}

	template <>
	inline std::array<vk::VertexInputAttributeDescription, 2>
		getAttributeDescriptions<Vertex3D>(uint32_t binding)
	{
		return {
			vk::VertexInputAttributeDescription{}
				.setBinding(binding)
				.setLocation(0)
				.setOffset(offsetof(Vertex3D, vertex))
				.setFormat(vk::Format::eR32G32B32Sfloat),
			vk::VertexInputAttributeDescription{}
				.setBinding(binding)
				.setLocation(1)
				.setOffset(offsetof(Vertex3D, color))
				.setFormat(vk::Format::eR32G32B32A32Sfloat)
		};
	}
}