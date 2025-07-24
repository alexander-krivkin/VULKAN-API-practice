#pragma once

#include <iostream>
#include <string>
#include <numeric>
#include <algorithm>

#include "shared.h"

#define FRACTION_2(a__) FractionVec2{ Fraction::convert(a__[0]), Fraction::convert(a__[1]) }
#define FRACTION_3(a__) FractionVec3{ Fraction::convert(a__[0]), Fraction::convert(a__[1]), Fraction::convert(a__[2]) }
#define GLM_VEC(a__) FLOAT(Fraction::convert(a__))
#define GLM_VEC_2(a__) glm::vec2{ FLOAT(Fraction::convert(a__[0])), FLOAT(Fraction::convert(a__[1])) }
#define GLM_VEC_3(a__) glm::vec3{ FLOAT(Fraction::convert(a__[0])), FLOAT(Fraction::convert(a__[1])), FLOAT(Fraction::convert(a__[2])) }


namespace ak
{
	struct Fraction
	{
		alignas(8) int64_t numerator { 0 };
		alignas(8) int64_t denominator { 1 };

		Fraction& operator=(const Fraction& rhs);
		Fraction operator=(int32_t num);
		Fraction operator=(double num);

		static Fraction simplify(const Fraction& arg);
		static Fraction simplify(int64_t num, int64_t denom);
		static Fraction simplifyAndSlash(int64_t num, int64_t denom);
		static Fraction simplifyAndSlash(const Fraction& arg);
		static Fraction convert(int32_t num, int32_t denom);
		static Fraction convert(int32_t num);
		static Fraction convert(double num);
		static double convert(const Fraction& arg);
	};

	bool operator==(const Fraction& lhs, const Fraction& rhs);
	bool operator!=(const Fraction& lhs, const Fraction& rhs);
	Fraction operator-(const Fraction& lhs);
	Fraction operator+(const Fraction& lhs, const Fraction& rhs);
	Fraction operator-(const Fraction& lhs, const Fraction& rhs);
	Fraction operator*(const Fraction& lhs, const Fraction& rhs);
	Fraction operator/(const Fraction& lhs, const Fraction& rhs);
	void operator+=(Fraction& lhs, const Fraction& rhs);
	void operator-=(Fraction& lhs, const Fraction& rhs);
	void operator*=(Fraction& lhs, const Fraction& rhs);
	void operator/=(Fraction& lhs, const Fraction& rhs);
	Fraction operator+(const Fraction& lhs, int32_t num);
	Fraction operator+(const Fraction& lhs, double num);
	Fraction operator-(const Fraction& lhs, int32_t num);
	Fraction operator-(const Fraction& lhs, double num);
	Fraction operator*(const Fraction& lhs, int32_t num);
	Fraction operator*(const Fraction& lhs, double num);
	Fraction operator/(const Fraction& lhs, int32_t num);
	Fraction operator/(const Fraction& lhs, double num);
	void operator+=(Fraction& lhs, int32_t num);
	void operator+=(Fraction& lhs, double num);
	void operator-=(Fraction& lhs, int32_t num);
	void operator-=(Fraction& lhs, double num);
	void operator*=(Fraction& lhs, int32_t num);
	void operator*=(Fraction& lhs, double num);
	void operator/=(Fraction& lhs, int32_t num);
	void operator/=(Fraction& lhs, double num);

	typedef std::array<Fraction, 2> FractionVec2;
	typedef std::array<Fraction, 3> FractionVec3;
}