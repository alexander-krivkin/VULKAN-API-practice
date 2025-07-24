#include "Fraction.h"


namespace ak
{
	Fraction& Fraction::operator=(const Fraction& rhs)
	{
		if (this == &rhs) { return *this; }

		numerator = rhs.numerator;
		denominator = rhs.denominator;

		return *this;
	}

	Fraction Fraction::operator=(int32_t num)
	{
		return convert(num);
	}

	Fraction Fraction::operator=(double num)
	{
		return convert(num);
	}

	Fraction Fraction::simplify(const Fraction& arg)
	{
		int64_t gcdValue = INT64(std::gcd(arg.numerator, arg.denominator));

		if (arg.denominator > 0)
		{
			return { arg.numerator / gcdValue, arg.denominator / gcdValue };
		}
		else
		{
			return { -(arg.numerator / gcdValue), -(arg.denominator / gcdValue) };
		}
	}

	Fraction Fraction::simplify(int64_t num, int64_t denom)
	{
		int64_t gcdValue = INT64(std::gcd(num, denom));

		if (denom > 0)
		{
			return { num / gcdValue, denom / gcdValue };
		}
		else
		{
			return { -(num / gcdValue), -(denom / gcdValue) };
		}
	}

	Fraction Fraction::simplifyAndSlash(int64_t num, int64_t denom)
	{
		auto ret = simplify(num, denom);
		auto maxAbsNumDenom = std::max(std::abs(ret.numerator), std::abs(ret.denominator));

		if (maxAbsNumDenom >= INT64(INT32_MAX))
		{
			auto excess = INT64(INT32_MAX) - maxAbsNumDenom;
			uint32_t excessDigit = INT32(ceil(log10(excess))) + 1;

			ret.numerator /= INT64(pow(10, excessDigit));
			ret.denominator /= INT64(pow(10, excessDigit));

			return simplify(ret.numerator, ret.denominator);
		}

		return ret;
	}

	Fraction Fraction::simplifyAndSlash(const Fraction& arg)
	{
		auto ret = simplify(arg);
		auto maxAbsNumDenom = std::max(std::abs(ret.numerator), std::abs(ret.denominator));

		if (maxAbsNumDenom >= INT64(INT32_MAX))
		{
			auto excess = INT64(INT32_MAX) - maxAbsNumDenom;
			uint32_t excessDigit = INT32(ceil(log10(excess))) + 1;

			ret.numerator /= INT64(pow(10, excessDigit));
			ret.denominator /= INT64(pow(10, excessDigit));

			return simplify(ret.numerator, ret.denominator);
		}

		return ret;
	}


	Fraction Fraction::convert(int32_t num, int32_t denom)
	{
		return simplifyAndSlash(INT64(num), INT64(denom));
	}

	Fraction Fraction::convert(int32_t num)
	{
		return { INT64(num), INT64(1) };
	}

	Fraction Fraction::convert(double num)
	{
		if (!num)
		{
			return Fraction{ 0, 1 };
		}
		else
		{
			std::string numStr = std::to_string(num);
			auto pos = numStr.find(".");
			if (pos != std::string::npos)
			{
				auto afterDotCount = numStr.substr(pos + 1).length();
				numStr.erase(pos, 1);
				auto numStrLength = numStr.length();

				if (numStrLength > 17)
				{
					numStr.erase(numStr.begin() + 17, numStr.end());
					afterDotCount -= (numStrLength - 17);
				}

				int64_t numerator = INT64(std::stoll(numStr));
				int64_t denominator = INT64(pow(10, afterDotCount));
				return simplifyAndSlash(numerator, denominator);
			}
			else
			{
				return { INT64(num), INT64(1) };
			}
		}
	}

	double Fraction::convert(const Fraction& arg)
	{
		return DOUBLE(arg.numerator) / DOUBLE(arg.denominator);
	}


	bool operator==(const Fraction& lhs, const Fraction& rhs)
	{
		auto lhsSymplified = Fraction::simplify(lhs);
		auto rhsSymplified = Fraction::simplify(rhs);

		return ((lhsSymplified.numerator == rhsSymplified.numerator) &&
			(lhsSymplified.denominator == rhsSymplified.denominator));
	}

	bool operator!=(const Fraction& lhs, const Fraction& rhs)
	{
		auto lhsSymplified = Fraction::simplify(lhs);
		auto rhsSymplified = Fraction::simplify(rhs);

		return ((lhsSymplified.numerator != rhsSymplified.numerator) ||
			(lhsSymplified.denominator != rhsSymplified.denominator));
	}

	Fraction operator-(const Fraction& lhs)
	{
		return Fraction::simplify(-lhs.numerator, lhs.denominator);
	}

	Fraction operator+(const Fraction& lhs, const Fraction& rhs)
	{
		auto numerator = lhs.numerator * rhs.denominator + rhs.numerator * lhs.denominator;
		auto denominator = rhs.denominator * lhs.denominator;

		return Fraction::simplifyAndSlash(numerator, denominator);
	}

	Fraction operator-(const Fraction& lhs, const Fraction& rhs)
	{
		auto numerator = lhs.numerator * rhs.denominator - rhs.numerator * lhs.denominator;
		auto denominator = rhs.denominator * lhs.denominator;

		return Fraction::simplifyAndSlash(numerator, denominator);
	}

	Fraction operator*(const Fraction& lhs, const Fraction& rhs)
	{
		auto numerator = lhs.numerator * rhs.numerator;
		auto denominator = rhs.denominator * lhs.denominator;

		return Fraction::simplifyAndSlash(numerator, denominator);
	}

	Fraction operator/(const Fraction& lhs, const Fraction& rhs)
	{
		auto numerator = lhs.numerator * rhs.denominator;
		auto denominator = lhs.denominator * rhs.numerator;

		return Fraction::simplifyAndSlash(numerator, denominator);
	}

	void operator+=(Fraction& lhs, const Fraction& rhs)
	{
		lhs = lhs + rhs;
	}

	void operator-=(Fraction& lhs, const Fraction& rhs)
	{
		lhs = lhs - rhs;
	}

	void operator*=(Fraction& lhs, const Fraction& rhs)
	{
		lhs = lhs * rhs;
	}

	void operator/=(Fraction& lhs, const Fraction& rhs)
	{
		lhs = lhs / rhs;
	}

	Fraction operator+(const Fraction& lhs, int32_t num)
	{
		auto rhs = Fraction::convert(num);
		return lhs + rhs;
	}

	Fraction operator+(const Fraction& lhs, double num)
	{
		auto rhs = Fraction::convert(num);
		return lhs + rhs;
	}

	Fraction operator-(const Fraction& lhs, int32_t num)
	{
		auto rhs = Fraction::convert(num);
		return lhs - rhs;
	}

	Fraction operator-(const Fraction& lhs, double num)
	{
		auto rhs = Fraction::convert(num);
		return lhs - rhs;
	}

	Fraction operator*(const Fraction& lhs, int32_t num)
	{
		auto rhs = Fraction::convert(num);
		return lhs * rhs;
	}

	Fraction operator*(const Fraction& lhs, double num)
	{
		auto rhs = Fraction::convert(num);
		return lhs * rhs;
	}

	Fraction operator/(const Fraction& lhs, int32_t num)
	{
		auto rhs = Fraction::convert(num);
		return lhs / rhs;
	}

	Fraction operator/(const Fraction& lhs, double num)
	{
		auto rhs = Fraction::convert(num);
		return lhs / rhs;
	}

	void operator+=(Fraction& lhs, int32_t num)
	{
		auto rhs = Fraction::convert(num);
		lhs = lhs + rhs;
	}

	void operator+=(Fraction& lhs, double num)
	{
		auto rhs = Fraction::convert(num);
		lhs = lhs + rhs;
	}

	void operator-=(Fraction& lhs, int32_t num)
	{
		auto rhs = Fraction::convert(num);
		lhs = lhs - rhs;
	}

	void operator-=(Fraction& lhs, double num)
	{
		auto rhs = Fraction::convert(num);
		lhs = lhs - rhs;
	}

	void operator*=(Fraction& lhs, int32_t num)
	{
		auto rhs = Fraction::convert(num);
		lhs = lhs * rhs;
	}

	void operator*=(Fraction& lhs, double num)
	{
		auto rhs = Fraction::convert(num);
		lhs = lhs * rhs;
	}

	void operator/=(Fraction& lhs, int32_t num)
	{
		auto rhs = Fraction::convert(num);
		lhs = lhs / rhs;
	}

	void operator/=(Fraction& lhs, double num)
	{
		auto rhs = Fraction::convert(num);
		lhs = lhs / rhs;
	}
}