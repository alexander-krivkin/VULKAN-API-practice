#pragma once

#include <array>
#include <vector>
#include <map>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shared.h"
#include "Fraction.h"


namespace ak::geometry
{
	template <typename T>
	class CoordinateSystem final
	{
	public:
		CoordinateSystem();

		inline explicit CoordinateSystem(std::array<T, 3> normals, glm::mat4 mattrix) :
			normalX_(normals[0]), normalY_(normals[1]),	normalZ_(normals[2]),
			mattrix_(mattrix)
		{
		}

		inline explicit CoordinateSystem(const std::array<T, 3>& normals, glm::mat4 mattrix) :
			normalX_(normals[0]), normalY_(normals[1]), normalZ_(normals[2]),
			mattrix_(mattrix)
		{
		}

		inline CoordinateSystem(const CoordinateSystem& rhs)
		{
			normalX_ = rhs.normalX_;
			normalY_ = rhs.normalY_;
			normalZ_ = rhs.normalZ_;
			mattrix_ = rhs.mattrix_;
		}

		~CoordinateSystem() {};

		inline CoordinateSystem& operator=(const CoordinateSystem& rhs)
		{
			if (this == &rhs) { return *this; }

			normalX_ = rhs.normalX_;
			normalY_ = rhs.normalY_;
			normalZ_ = rhs.normalZ_;
			mattrix_ = rhs.mattrix_;

			return *this;
		}


		inline void reset();
		inline CoordinateSystem& normalizeBaseX();
		inline CoordinateSystem& normalizeBaseY();
		inline CoordinateSystem& normalizeBaseZ();

		//inline CoordinateSystem rotateX(Fraction angle);
		//inline CoordinateSystem rotateY(Fraction angle);
		//inline CoordinateSystem rotateZ(Fraction angle);
		inline CoordinateSystem& rotateX(float angle);
		inline CoordinateSystem& rotateY(float angle);
		inline CoordinateSystem& rotateZ(float angle);

		inline CoordinateSystem getRotatedX(float angle) const;
		inline CoordinateSystem getRotatedY(float angle) const;
		inline CoordinateSystem getRotatedZ(float angle) const;

		inline T getNormalX() const { return normalX_; }
		inline T getNormalY() const { return normalY_; }
		inline T getNormalZ() const { return normalZ_; }
		inline std::array<T, 3> getNormals() const
		{
			return std::array<T, 3>{ normalX_, normalY_, normalZ_ };
		}
		inline glm::mat4 getMattrix() const
		{
			return mattrix_;
		}

	private:
		T normalX_;
		T normalY_;
		T normalZ_;
		glm::mat4 mattrix_;
	};

	template <>
	inline CoordinateSystem<FractionVec3>::CoordinateSystem() :
		normalX_({ { 1, 1 }, { 0, 1 }, { 0, 1 } }),
		normalY_({ { 0, 1 }, { 1, 1 }, { 0, 1 } }),
		normalZ_({ { 0, 1 }, { 0, 1 }, { 1, 1 } }),
		mattrix_(glm::mat4(1.0f))
	{
	}

	template <>
	inline CoordinateSystem<glm::vec3>::CoordinateSystem() :
		normalX_({ 1.0f, 0.0f, 0.0f }),
		normalY_({ 0.0f, 1.0f, 0.0f }),
		normalZ_({ 0.0f, 0.0f, 1.0f }),
		mattrix_(glm::mat4(1.0f))
	{
	}

	template <>
	inline void CoordinateSystem<FractionVec3>::reset()
	{
		normalX_ = { Fraction{ 1, 1 }, Fraction{ 0, 1 }, Fraction{ 0, 1 } };
		normalY_ = { Fraction{ 0, 1 }, Fraction{ 1, 1 }, Fraction{ 0, 1 } };
		normalZ_ = { Fraction{ 0, 1 }, Fraction{ 0, 1 }, Fraction{ 1, 1 } };
		mattrix_ = glm::mat4(1.0f);
	}

	template <>
	inline void CoordinateSystem<glm::vec3>::reset()
	{
		normalX_ = { 1.0f, 0.0f, 0.0f };
		normalY_ = { 0.0f, 1.0f, 0.0f };
		normalZ_ = { 0.0f, 0.0f, 1.0f };
		mattrix_ = glm::mat4(1.0f);
	}

	template <>
	inline CoordinateSystem<glm::vec3>& CoordinateSystem<glm::vec3>::normalizeBaseX()
	{
		normalX_ = glm::normalize(normalX_);
		normalY_ = glm::normalize(glm::cross(normalZ_, normalX_));
		normalZ_ = glm::normalize(glm::cross(normalX_, normalY_));

		return *this;
	}

	template <>
	inline CoordinateSystem<glm::vec3>& CoordinateSystem<glm::vec3>::normalizeBaseY()
	{
		normalY_ = glm::normalize(normalY_);
		normalZ_ = glm::normalize(glm::cross(normalX_, normalY_));
		normalX_ = glm::normalize(glm::cross(normalY_, normalZ_));

		return *this;
	}

	template <>
	inline CoordinateSystem<glm::vec3>& CoordinateSystem<glm::vec3>::normalizeBaseZ()
	{
		normalZ_ = glm::normalize(normalZ_);
		normalX_ = glm::normalize(glm::cross(normalY_, normalZ_));
		normalY_ = glm::normalize(glm::cross(normalZ_, normalX_));

		return *this;
	}

	template <>
	inline CoordinateSystem<glm::vec3>& CoordinateSystem<glm::vec3>::rotateX(float angle)
	{
		mattrix_ = glm::rotate(mattrix_, angle, normalX_);
		normalY_ = mattrix_ * glm::vec4(normalY_, 1.0f);
		normalZ_ = mattrix_ * glm::vec4(normalZ_, 1.0f);
		
		return *this;
	}

	template <>
	inline CoordinateSystem<glm::vec3>& CoordinateSystem<glm::vec3>::rotateY(float angle)
	{
		mattrix_ = glm::rotate(mattrix_, angle, normalY_);
		normalX_ = mattrix_ * glm::vec4(normalX_, 1.0f);
		normalZ_ = mattrix_ * glm::vec4(normalZ_, 1.0f);

		return *this;
	}

	template <>
	inline CoordinateSystem<glm::vec3>& CoordinateSystem<glm::vec3>::rotateZ(float angle)
	{
		mattrix_ = glm::rotate(mattrix_, angle, normalZ_);
		normalX_ = mattrix_ * glm::vec4(normalX_, 1.0f);
		normalY_ = mattrix_ * glm::vec4(normalY_, 1.0f);

		return *this;
	}

	template <>
	inline CoordinateSystem<glm::vec3> CoordinateSystem<glm::vec3>::getRotatedX(float angle) const
	{
		CoordinateSystem<glm::vec3> ret{};
		ret.mattrix_ = glm::rotate(ret.mattrix_, angle, normalX_);

		ret.normalX_ = normalX_;
		ret.normalY_ = ret.mattrix_ * glm::vec4(normalY_, 1.0f);
		ret.normalZ_ = ret.mattrix_ * glm::vec4(normalZ_, 1.0f);

		return ret;
	}

	template <>
	inline CoordinateSystem<glm::vec3> CoordinateSystem<glm::vec3>::getRotatedY(float angle) const
	{
		CoordinateSystem<glm::vec3> ret{};
		ret.mattrix_ = glm::rotate(ret.mattrix_, angle, normalY_);

		ret.normalY_ = normalY_;
		ret.normalX_ = ret.mattrix_ * glm::vec4(normalX_, 1.0f);
		ret.normalZ_ = ret.mattrix_ * glm::vec4(normalZ_, 1.0f);

		return ret;
	}

	template <>
	inline CoordinateSystem<glm::vec3> CoordinateSystem<glm::vec3>::getRotatedZ(float angle) const
	{
		CoordinateSystem<glm::vec3> ret{};
		ret.mattrix_ = glm::rotate(ret.mattrix_, angle, normalZ_);

		ret.normalZ_ = normalZ_;
		ret.normalX_ = ret.mattrix_ * glm::vec4(normalX_, 1.0f);
		ret.normalY_ = ret.mattrix_ * glm::vec4(normalY_, 1.0f);

		return ret;
	}
}