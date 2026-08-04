#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string_view>
#include <string>
#include <optional>
#include <iostream>

namespace Context
{
	enum ErrorSeverity
	{
		Info,
		Warning,
		Error
	};

	struct RuntimeError
	{
		std::string message{};
		ErrorSeverity severity{ ErrorSeverity::Error };
		size_t charPosition{ 0 };
		size_t length{ 1 };

		void print() const
		{
			std::cout << message << "\n";
		}
	};
}

struct Token
{
	enum Type
	{
		Identifier,
		LParen,
		RParen,
		Number,
		Comma,
		Equals
	};

	Type type{};
	std::string_view lexeme{};

	size_t charPosition{};
};

namespace Lexer
{
	extern std::vector<Token> tokens;
}

auto tokenizer(const std::string& input, std::optional<Context::RuntimeError>& diag) -> void;
auto convertTokenTo_string_view(Token::Type type)					  -> std::string_view;
auto printTokens(const std::vector<Token>& tokens)					  -> void;

auto isAlnum(char ch) -> bool;
auto isDigit(char ch) -> bool;
auto isUpper(char ch) -> bool;
auto isAlpha(char ch) -> bool;

#endif