#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace ak::geometry
{
    struct Point2D
    {
        glm::vec2 vertex{};
        glm::vec4 color{};
    };

    struct Point3D
    {
        glm::vec3 vertex{};
        glm::vec4 color{};
    };

    struct Segment2D
    {
        std::array<Point2D, 2> points{};
    };

    struct Segment3D
    {
        std::array<Point3D, 2> points{};
    };

    struct Polyline2D
    {
        std::vector<glm::vec2> vertices{};
        glm::vec4 color{};
    };

    struct Polyline3D
    {
        std::vector<glm::vec3> vertices{};
        glm::vec4 color{};
    };

    struct UniformTransformation
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
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
        getBindDescription<Point2D>(uint32_t binding)
    {
        return vk::VertexInputBindingDescription{}
            .setBinding(binding)
            .setStride(sizeof(Point2D))
            .setInputRate(vk::VertexInputRate::eVertex);
    }

    template <>
    inline vk::VertexInputBindingDescription
        getBindDescription<Point3D>(uint32_t binding)
    {
        return vk::VertexInputBindingDescription{}
            .setBinding(binding)
            .setStride(sizeof(Point3D))
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
        getAttributeDescriptions<Point2D>(uint32_t binding)
    {
        return {
            vk::VertexInputAttributeDescription{}
                .setBinding(binding)
                .setLocation(0)
                .setOffset(offsetof(Point2D, vertex))
                .setFormat(vk::Format::eR32G32Sfloat),
            vk::VertexInputAttributeDescription{}
                .setBinding(binding)
                .setLocation(1)
                .setOffset(offsetof(Point2D, color))
                .setFormat(vk::Format::eR32G32B32A32Sfloat)
        };
    }

    template <>
    inline std::array<vk::VertexInputAttributeDescription, 2>
        getAttributeDescriptions<Point3D>(uint32_t binding)
    {
        return {
            vk::VertexInputAttributeDescription{}
                .setBinding(binding)
                .setLocation(0)
                .setOffset(offsetof(Point3D, vertex))
                .setFormat(vk::Format::eR32G32Sfloat),
            vk::VertexInputAttributeDescription{}
                .setBinding(binding)
                .setLocation(1)
                .setOffset(offsetof(Point3D, color))
                .setFormat(vk::Format::eR32G32B32A32Sfloat)
        };
    }

}