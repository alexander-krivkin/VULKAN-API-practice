#pragma once

#define BOOL(x__) static_cast<bool>(x__)
#define FLOAT(x__) static_cast<float>(x__)
#define DOUBLE(x__) static_cast<double>(x__)
#define INT16(x__) static_cast<int16_t>(x__)
#define INT32(x__) static_cast<int32_t>(x__)
#define INT64(x__) static_cast<int64_t>(x__)
#define UINT16(x__) static_cast<uint16_t>(x__)
#define UINT32(x__) static_cast<uint32_t>(x__)
#define UINT64(x__) static_cast<uint64_t>(x__)

#define RGB_BLACK glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
#define RGB_GRAY glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f }
#define RGB_RED   glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f }
#define RGB_GREEN glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f }
#define RGB_BLUE  glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f }
#define RGB_WHITE glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }


namespace ak
{
	constexpr uint32_t numFramesInFlight{ 2 };
	constexpr uint32_t numTopologyTypes{ 3 };
	constexpr uint32_t numShaders{ 2 };
	constexpr uint32_t numObjects{ 3 };
	constexpr uint32_t numFragments{ 32 };

	enum class TopologyTypeFlag : uint32_t
	{
		ePoints = 0,
		eLines = 1,
		eTriangles = 2,
		eMax = numTopologyTypes
	};

	enum class ShaderFlag : uint32_t
	{
		eVertex = 0,
		eFragment = 1,
		eMax = numShaders
	};

	enum class ObjectFlag : uint32_t
	{
		eGrid = 0,
		eScene = 1,
		eCrosshair = 2,
		eMax = numObjects
	};


	enum class KeyboardMouseFlagBits : uint32_t
	{
		eKeyLeftShift = 0x00000001,
		eKeyRightShift = 0x00000002,
		eKeyLeftCtrl = 0x00000004,
		eKeyRightCtrl = 0x00000008,
		eKeyLeftAlt = 0x00000010,
		eKeyRightAlt = 0x00000020,
		eMouseLeft = 0x00000040,
		eMouseMiddle = 0x00000080,
		eMouseRight = 0x00000100
	};

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator|(KeyboardMouseFlagBits lhs, KeyboardMouseFlagBits rhs)
	{
		return (static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(lhs) |
			static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator|(int lhs, KeyboardMouseFlagBits rhs)
	{
		return (lhs | static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator|(KeyboardMouseFlagBits lhs, int rhs)
	{
		return (static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(lhs) | rhs);
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator&(KeyboardMouseFlagBits lhs, KeyboardMouseFlagBits rhs)
	{
		return (static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(lhs) &
			static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator&(int lhs, KeyboardMouseFlagBits rhs)
	{
		return (lhs & static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator&(KeyboardMouseFlagBits lhs, int rhs)
	{
		return (static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(lhs) & rhs);
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator^(KeyboardMouseFlagBits lhs, KeyboardMouseFlagBits rhs)
	{
		return (static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(lhs) ^
			static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator^(int lhs, KeyboardMouseFlagBits rhs)
	{
		return (lhs ^ static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator^(KeyboardMouseFlagBits lhs, int rhs)
	{
		return (static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(lhs) ^ rhs);
	}

	inline constexpr std::underlying_type_t<KeyboardMouseFlagBits>
		operator~(KeyboardMouseFlagBits lhs)
	{
		return ~(static_cast<std::underlying_type_t<KeyboardMouseFlagBits>>(lhs));
	}
}