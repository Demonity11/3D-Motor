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

std::optional<ParseResult> parser(const std::vector<Token>& tokens, std::optional<Context::RuntimeError>& diag, size_t tp)
{
    using Context::RuntimeError;
    using Context::ErrorSeverity;

    if (tokens.empty())
    {
        diag = RuntimeError
        {
            "Parser Warning at col 0: Input is empty.",
            ErrorSeverity::Warning,
            0,
            0
        };
  
        return std::nullopt;
    }

    static int parserCalls{ 0 };

    using Parser::nodes;

    const Token* token{ &tokens[tp] };

    if (token->type == Token::Equals)
    {
        diag = RuntimeError
        {
            "Parser Error at col " + std::to_string(token->charPosition + 1) + ": Unexpected assignment operator(=).",
            ErrorSeverity::Error,
            token->charPosition,
            1
        };

        Parser::nodes.clear();
        return std::nullopt;
    }

    if (token->type == Token::Number)
    {
        int myIdx{ static_cast<int>(nodes.size()) };
        nodes.push_back({ Node::Literal, token->lexeme, token->charPosition });

        return ParseResult{ tp + 1, myIdx };
    }

    if (token->type == Token::Identifier)
    {
        std::optional<std::string> targetName{ std::nullopt };

        if (tp == 0 && tp + 3 < tokens.size() &&
            tokens[tp + 1].type == Token::Equals &&
            tokens[tp + 2].type == Token::Identifier &&
            tokens[tp + 3].type == Token::LParen)
        {
            targetName = std::string(token->lexeme);
            for (const auto& func : Context::funcOverloads)
            {
                if (*targetName == func.name)
                {
                    diag = RuntimeError
                    {
                        "Parser Error at col " + std::to_string(token->charPosition + 1) + ": '" + *targetName + "' is a reserved keyword.",
                        ErrorSeverity::Error,
                        token->charPosition,
                        targetName->length()
                    };

                    return std::nullopt;
                }
            }
            tp += 2;
            token = &tokens[tp];
        }

        if (tp + 1 < tokens.size() && tokens[tp + 1].type == Token::LParen)
        {
            if (tp + 2 < tokens.size() && tokens[tp + 2].type == Token::RParen)
            {
                diag = RuntimeError
                {
                    "Parser Error at col " + std::to_string(token[tp + 1].charPosition + 1) + ": '" + std::string(token->lexeme) + "' function is empty.",
                    ErrorSeverity::Error,
                    tokens[tp + 1].charPosition,
                    1
                };

                return std::nullopt;
            }

            int parentIdx{ static_cast<int>(nodes.size()) };
            if (targetName)
            {
                nodes.push_back({ Node::Function, token->lexeme, token->charPosition, *targetName});
            }

            else
            {
                nodes.push_back({ Node::Function, token->lexeme, token->charPosition });
            }

            tp += 2;
            size_t childCount{ 0 };
            int lParenCount{ 1 };
            int rParenCount{ 0 };

            while (tp < tokens.size() && tokens[tp].type != Token::RParen)
            {
                ++parserCalls;
                std::optional<ParseResult> childResult{ parser(tokens, diag, tp) };
                --parserCalls;

                if (!childResult.has_value())
                {
                    nodes.clear();
                    return std::nullopt;
                }

                if (childCount > 2)
                {
                    diag = RuntimeError
                    {
                        "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Argument overflow.", 
                        ErrorSeverity::Error,
                        tokens[tp].charPosition,
                        tokens[tp].lexeme.length()
                    };

                    nodes.clear();

                    return std::nullopt;
                }

                nodes[parentIdx].children[childCount++] = childResult->nodeIdx;

                tp = childResult->nextTP;

                if (tp < tokens.size() && tokens[tp].type == Token::RParen) ++rParenCount;

                if (tp < tokens.size() && tokens[tp].type == Token::Comma)
                {
                    if (tp + 1 < tokens.size() && tokens[tp + 1].type == Token::Comma)
                    {
                        diag = RuntimeError
                        {
                            "Parser Error at col " + std::to_string(tokens[tp + 1].charPosition + 1) + ": Misplaced comma.",
                            ErrorSeverity::Error,
                            tokens[tp + 1].charPosition,
                            1
                        };

                        nodes.clear();

                        return std::nullopt;
                    }

                    tp++;
                }
            }

            if (lParenCount != rParenCount)
            {
                if (tokens[tp - 1].type == Token::Comma)
                {
                    diag = RuntimeError
                    {
                        "Parser Error at col " + std::to_string(tokens[tp - 1].charPosition + 1) + ": Unexpected trailing comma.",
                        ErrorSeverity::Error,
                        tokens[tp - 1].charPosition,
                        1
                    };

                    nodes.clear();

                    return std::nullopt;
                }

                diag = RuntimeError
                {
                    "Parser Error at col " + std::to_string(tokens[tp - 1].charPosition + 1) + ": Unmatched parenthesis.",
                    ErrorSeverity::Error,
                    tokens[tp - 1].charPosition,
                    tokens[tp - 1].lexeme.length()
                };

                nodes.clear();

                return std::nullopt;
            }

            tp++;

            if (parserCalls == 0 && tp < tokens.size())
            {
                diag = RuntimeError
                {
                    "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Unexpected trailing tokens.",
                    ErrorSeverity::Error,
                    tokens[tp].charPosition,
                    tokens[tp].lexeme.length()
                };

                nodes.clear();

                return std::nullopt;
            }

            else if (parserCalls > 0 && tp < tokens.size() && (tokens[tp].type == Token::Identifier || tokens[tp].type == Token::Number) && tokens[tp - 1].type != Token::Comma)
            {
                diag = RuntimeError
                {
                    "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Missing comma.",
                    ErrorSeverity::Error,
                    tokens[tp].charPosition,
                    tokens[tp].lexeme.length()
                };

                nodes.clear();

                return std::nullopt;
            }

            return ParseResult{ tp, parentIdx };
        }

        else
        {
            int myIdx = static_cast<int>(nodes.size());
            nodes.push_back({ Node::Variable, token->lexeme, token->charPosition });

            return ParseResult{ tp + 1, myIdx };
        }
    }

    diag = RuntimeError
    {
        "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Unexpected token.",
        ErrorSeverity::Error,
        tokens[tp].charPosition,
        tokens[tp].lexeme.length()
    };

    return std::nullopt;
}
