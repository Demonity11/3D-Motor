#include "lexer.h"
#include "scene/Context.h"
#include <iostream>
#include <cctype>

namespace Lexer
{
	std::vector<Token> tokens{};
}

constexpr std::string_view convertTokenTo_string_view(Token::Type type)
{
	switch (type)
	{
	case Token::Identifier: return "Identifier";
	case Token::Equals: return "Equals";
	case Token::BiOperator: return "BiOperator";
	case Token::LParen: return "LParen";
	case Token::RParen: return "RParen";
	case Token::Number: return "Number";
	case Token::Comma: return "Comma";
	}

	return "???";
}

void printTokens(const std::vector<Token>& tokens)
{
	for (size_t pos{ 0 }; pos < tokens.size(); ++pos)
	{
		const Token& token{ tokens[pos] };

		std::cout << "Pos: " << pos << "\t" << convertTokenTo_string_view(token.type) << ": " << token.lexeme << "\n";
	}
}

bool isAlnum(char ch) { return std::isalnum(static_cast<unsigned char>(ch)); }

bool isDigit(char ch) { return std::isdigit(static_cast<unsigned char>(ch)); }

bool isUpper(char ch) { return std::isupper(static_cast<unsigned char>(ch)); }

bool isAlpha(char ch) { return std::isalpha(static_cast<unsigned char>(ch)); }

void tokenizer(const std::string& input, std::optional<Context::RuntimeError>& diag)
{
	using Lexer::tokens;
	using Context::RuntimeError;
	using Context::ErrorSeverity;

	if (input.empty())
	{
		diag = { "Lexer Warning at col 0: Input field is empty. Please, type a command.", ErrorSeverity::Warning, 0, 0 };
		return;
	}

	size_t l{ 0 };
	while (l < input.size())
	{
		char c{ input[l] };

		if (c == ' ' || c == '\n' || c == '\t')
		{
			l++;
			continue;
		}

		else if (c == '(')
		{
			tokens.push_back({ Token::LParen, std::string_view(&input[l], 1), l });
		}

		else if (c == ')')
		{
			tokens.push_back({ Token::RParen, std::string_view(&input[l], 1), l });
		}

		else if (c == ',')
		{
			tokens.push_back({ Token::Comma, std::string_view(&input[l], 1), l });
		}

		else if (c == '=')
		{
			tokens.push_back({ Token::Equals, std::string_view(&input[l], 1), l });
		}
		
		else if (c == '+' || c == '*')
		{
			tokens.push_back({ Token::BiOperator, std::string_view(&input[l], 1), l });
		}

		else if (c == '-' && l + 1 < input.length() && input[l + 1] != '.' && input[l + 1] != '-' && !isDigit(input[l + 1]))
		{
			tokens.push_back({ Token::BiOperator, std::string_view(&input[l], 1), l });
		}

		else if (isAlpha(c))
		{
			size_t start{ l };
			while (l < input.size() && (isAlnum(input[l]) || input[l] == '_'))
			{
				l++;
			}

			size_t end{ l - start };

			tokens.push_back({ Token::Identifier, std::string_view(&input[start], end), start });
			continue;
		}

		else if (isDigit(c) || c == '-' || c == '.')
		{
			int pointCount{ 0 };
			size_t start{ l };

			if (c == '-')
			{
				if (l + 1 >= input.size() || (!isDigit(input[l + 1]) && input[l + 1] != '.'))
				{
					diag = RuntimeError
					{
						"Lexer Error at col " + std::to_string(start + 1) + ": '-' must be followed by a digit.",
						ErrorSeverity::Error,
						start, 
						1      
					};

					tokens.clear();
					return;
				}
				++l;
			}

			while (l < input.size() && (isDigit(input[l]) || input[l] == '.'))
			{
				c = input[l];
				if (c == '.') 
				{
					++pointCount;
					if (pointCount > 1)
					{
						size_t errLength{ (l - start) + 1 };

						diag = RuntimeError
						{
							"Lexer Error at col " + std::to_string(start + 1) + ": Numbers can only have one decimal point.",
							ErrorSeverity::Error,
							start,     
							errLength
						};

						tokens.clear();
						return;
					}
				}

				l++;
			}

			size_t end{ l - start };

			if (end == 0 && l < input.size() && input[l] == '.')
			{
				diag = RuntimeError
				{
					"Lexer Error at col " + std::to_string(l + 1) + ": A decimal point must be preceded or followed by a number.",
					ErrorSeverity::Error,
					l,
					1
				};

				tokens.clear();
				return;
			}

			tokens.push_back({ Token::Number, std::string_view(&input[start], end), start });
			continue;
		}

		else
		{
			diag = RuntimeError
			{
				"Lexer Error at col " + std::to_string(l + 1) + ": Character '" + c + "' is not valid.",
				ErrorSeverity::Error,
				l,
				1
			};

			tokens.clear();
			return;
		}

		l++;
	}
}