#pragma once

#include <vector>
#include <array>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>


namespace ak::graphics
{
    // Потом это превратится в класс-наследник. Пока это структуры
    struct Point
    {
        glm::vec2 vertex{};
        glm::vec4 color{};
    };

    struct Polyline
    {
        std::vector<glm::vec2> vertices{};
        glm::vec4 color{};
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
        getBindDescription<Point>(uint32_t binding)
    {
        return vk::VertexInputBindingDescription{}
            .setBinding(binding)
            .setStride(sizeof(Point))
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
        getAttributeDescriptions<Point>(uint32_t binding)
    {
        return {
            vk::VertexInputAttributeDescription{}
                .setBinding(binding)
                .setLocation(0)
                .setOffset(offsetof(Point, vertex))
                .setFormat(vk::Format::eR32G32Sfloat),
            vk::VertexInputAttributeDescription{}
                .setBinding(binding)
                .setLocation(1)
                .setOffset(offsetof(Point, color))
                .setFormat(vk::Format::eR32G32B32A32Sfloat)
        };
    }
}