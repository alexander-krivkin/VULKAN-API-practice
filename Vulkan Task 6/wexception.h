#pragma once

#include <string>


namespace ak
{
	class wexception final
	{
	public:
		wexception() : code_(-1), message_(L"") {}
		explicit wexception(const wexception& other) : code_(other.code_), message_(other.message_) {}
		explicit wexception(int code) : code_(code), message_(L"") {}
		explicit wexception(std::wstring message) : code_(-1), message_(message) {}
		explicit wexception(int code, std::wstring message) : code_(code), message_(message) {}
		wexception& operator=(const wexception& other)
		{
			if (this == &other) return *this;
			code_ = other.code_;
			message_ = other.message_;
		}
		~wexception() {}

		int code() const { return code_; }
		const wchar_t* what() const { return message_.c_str(); }

	private:
		int code_{ -1 };
		std::wstring message_{};
	};
}