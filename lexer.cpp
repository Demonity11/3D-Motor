#include "lexer.h"
#include "Context.h"
#include <iostream>
#include <cctype>

namespace Lexer
{
	std::vector<Token> tokens{};
}

std::string_view convertTokenTo_string_view(Token::Type type)
{
	switch (type)
	{
	case Token::Identifier: return "Identifier";
	case Token::Equals: return "Equals";
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

void tokenizer(const std::string& input)
{
	using Lexer::tokens;

	if (input.empty())
	{
		std::cerr << "LEXER::ERROR::INPUT_IS_EMPTY\n";
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
			tokens.push_back({ Token::LParen, std::string_view(&input[l], 1) });
		}

		else if (c == ')')
		{
			tokens.push_back({ Token::RParen, std::string_view(&input[l], 1) });
		}

		else if (c == ',')
		{
			tokens.push_back({ Token::Comma, std::string_view(&input[l], 1) });
		}

		else if (c == '=')
		{
			tokens.push_back({ Token::Equals, std::string_view(&input[l], 1) });
		}

		else if (isAlpha(c))
		{
			size_t start{ l };
			while (isAlnum(input[l]))
			{
				l++;
			}

			size_t end{ l - start };

			tokens.push_back({ Token::Identifier, std::string_view(&input[start], end) });
			continue;
		}

		else if (isDigit(c) || c == '-' || c == '.')
		{
			int pointCount{ 0 };
			size_t start{ l };

			if (c == '-' || c == '.')
			{
				if (c == '.') ++pointCount;

				if (l + 1 < input.size()) 
				{
					if (isDigit(input[l + 1]))
					{
						++l;
					}
					else
					{
						std::cerr << "LEXER::ERROR<" << input[l + 1] << ">IS_NOT_A_NUMBER\n";
						tokens.clear();
						return;
					}
				}
			}

			while (isDigit(input[l]) || input[l] == '.')
			{
				c = input[l];

				if (c == '.') pointCount++;
				
				if (pointCount > 1)
				{
					std::cerr << "LEXER::ERROR::NUMBER_HAS_MORE_THAN_ONE_FLOATING_POINT\n";
					tokens.clear();
					return;
				}

				l++;
			}

			size_t end{ l - start };

			tokens.push_back({ Token::Number, std::string_view(&input[start], end) });
			continue;
		}

		else
		{
			std::cerr << "LEXER::ERROR::CHARACTER<" << c << ">IS_NOT_VALID\n";
			tokens.clear();
			return;
		}

		l++;
	}
}