#pragma once

#include <string>


namespace ak
{
	class wexception final
	{
	public:
		wexception() : code_(-1), message_(L"") {};
		wexception(const wexception&) = delete;
		explicit wexception(int code) : code_(code), message_(L"") {};
		explicit wexception(std::wstring message) : code_(-1), message_(message) {};
		explicit wexception(int code, std::wstring message) : code_(code), message_(message) {};
		wexception& operator=(const wexception&) = delete;
		~wexception() {};

		int code() const { return code_; }
		const wchar_t* what() const { return message_.c_str(); }

	private:
		int code_{ -1 };
		std::wstring message_{};
	};
}