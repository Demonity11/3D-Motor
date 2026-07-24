#include "parser.h"
#include <iostream>
#include "Context.h"

namespace Parser
{
	std::vector<Node> nodes{};
}

std::string_view convertNodeTo_string_view(Node::Type type)
{
	switch (type)
	{
	case Node::Function: return "Function";
	case Node::Variable: return "Variable";
	case Node::Literal: return "Literal";
	}

	return "???";
}

void printNodes(const std::vector<Node>& nodes)
{
	for (size_t pos{ 0 }; pos < nodes.size(); ++pos)
	{
		const Node& node{ nodes[pos] };

		std::cout << "Pos: " << pos << "\t" << convertNodeTo_string_view(node.type) << " : " << node.content << " : ["
			      << node.children[0] << ", " << node.children[1] << ", " << node.children[2] << "]\n";
	}
}

std::optional<ParseResult> parser(const std::vector<Token>& tokens, size_t tp)
{
    if (tokens.empty())
    {
        std::cerr << "PARSER::ERROR::TOKENS_EMPTY\n";
        return {};
    }

    static int parserCalls{ 0 };

    using Parser::nodes;

    const Token& token{ tokens[tp] };

    if (token.type == Token::Equals)
    {
        std::cerr << "ERROR::SYNTAX::UNEXPECTED_ASSIGNMENT_OPERATOR\n";
        Parser::nodes.clear();
        return {};
    }

    if (token.type == Token::Number)
    {
        int myIdx{ static_cast<int>(nodes.size()) };
        nodes.push_back({ Node::Literal, token.lexeme });

        return ParseResult{ tp + 1, myIdx };
    }

    if (token.type == Token::Identifier)
    {
        std::optional<std::string> targetName{ std::nullopt };

        if (tp == 0 && tp + 3 < tokens.size() &&
            tokens[tp + 1].type == Token::Equals &&
            tokens[tp + 2].type == Token::Identifier &&
            tokens[tp + 3].type == Token::LParen)
        {
            targetName = std::string(token.lexeme);
            for (const auto& func : Context::function)
            {
                if (*targetName == func.name)
                {
                    std::cerr << "ERROR::SYNTAX<" << *targetName << ">::IS_RESERVED\n";
                    return {};
                }
            }
            tp += 2;
        }

        if (tp + 1 < tokens.size() && tokens[tp + 1].type == Token::LParen)
        {
            if (tp + 2 < tokens.size() && tokens[tp + 2].type == Token::RParen)
            {
                std::cerr << "ERROR::SYNTAX::" << token.lexeme << "_IS_EMPTY\n";

                return {};
            }

            int parentIdx{ static_cast<int>(nodes.size()) };
            if (targetName)
            {
                nodes.push_back({ Node::Function, tokens[tp].lexeme, *targetName});
            }

            else
            {
                nodes.push_back({ Node::Function, token.lexeme });
            }

            tp += 2;
            size_t childCount{ 0 };
            int lParenCount{ 1 };
            int rParenCount{ 0 };

            while (tp < tokens.size() && tokens[tp].type != Token::RParen)
            {
                ++parserCalls;
                std::optional<ParseResult> childResult{ parser(tokens, tp) };
                --parserCalls;

                if (!childResult.has_value())
                {
                    nodes.clear();
                    return {};
                }

                if (childCount > 2)
                {
                    std::cerr << "ERROR::SYNTAX::ARGUMENT_OVERFLOW\n";
                    nodes.clear();

                    return {};
                }

                nodes[parentIdx].children[childCount++] = childResult->nodeIdx;

                tp = childResult->nextTP;

                if (tp < tokens.size() && tokens[tp].type == Token::RParen) ++rParenCount;

                if (tp < tokens.size() && tokens[tp].type == Token::Comma)
                {
                    if (tp + 1 < tokens.size() && tokens[tp + 1].type == Token::Comma)
                    {
                        std::cerr << "ERROR::SYNTAX::MISPLACED_COMMA\n";
                        nodes.clear();

                        return {};
                    }

                    tp++;
                }
            }

            if (lParenCount != rParenCount)
            {
                if (tokens[tp - 1].type == Token::Comma)
                {
                    std::cerr << "ERROR::SYNTAX::UNEXPECTED_TRAILING_COMMA\n";
                    nodes.clear();

                    return {};
                }

                std::cerr << "ERROR::SYNTAX::UNMATCHED_PARENTHESIS\n";
                nodes.clear();

                return {};
            }

            tp++;

            if (parserCalls == 0 && tp < tokens.size())
            {
                std::cerr << "ERROR::SYNTAX::UNEXPECTED_TRAILING_TOKENS\n";
                nodes.clear();

                return {};
            }

            else if (parserCalls > 0 && tp < tokens.size() && (tokens[tp].type == Token::Identifier || tokens[tp].type == Token::Number) && tokens[tp - 1].type != Token::Comma)
            {
                std::cerr << "ERROR::SYNTAX::MISSING_COMMA\n";
                nodes.clear();

                return {};
            }

            return ParseResult{ tp, parentIdx };
        }

        else if (tp + 1 < tokens.size() && tokens[tp + 1].type == Token::Equals)
        {
            int myIdx = static_cast<int>(nodes.size());
            nodes.push_back({ Node::Variable, token.lexeme });

            return ParseResult{ tp + 1, myIdx };
        }

        else
        {
            int myIdx = static_cast<int>(nodes.size());
            nodes.push_back({ Node::Variable, token.lexeme });

            return ParseResult{ tp + 1, myIdx };
        }
    }

    std::cerr << "ERROR::SYNTAX::UNEXPECTED_TOKEN\n";

    return {};
}
