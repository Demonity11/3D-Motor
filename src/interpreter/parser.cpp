#include "parser.h"
#include <iostream>
#include "scene/Context.h"

namespace Parser
{
	std::vector<Node> nodes{};
}

constexpr std::string_view convertNodeTo_string_view(Node::Type type)
{
	switch (type)
	{
	case Node::Function: return "Function";
	case Node::Variable: return "Variable";
    case Node::Literal: return "Literal";
    case Node::Exp1: return "Exp1";
    case Node::Exp2: return "Exp2";
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

std::optional<ParseResult> parseChainedOperator(const std::vector<Token>& tokens, Diag& diag, OptName& targetName, size_t tp, int parentIdx)
{
    using Context::RuntimeError;
    using Context::ErrorSeverity;

    if (tp >= tokens.size())
    {
        diag = RuntimeError
        {
            "Parser Error at col " + std::to_string(tokens.back().charPosition + 1) + ": Expected right operand after operator '" + std::string(tokens.back().lexeme) + "'.",
            ErrorSeverity::Error,
            tokens.back().charPosition,
            tokens.back().lexeme.length()
        };
        return std::nullopt;
    }

    if (tokens[tp].type == Token::BiOperator)
    {
        size_t opPos{ tokens[tp].charPosition };
        std::string opLexeme{ tokens[tp].lexeme };

        Node chainedOperator{ (tokens[tp].lexeme == "*" ? Node::Exp2 : Node::Exp1), tokens[tp].lexeme, opPos };

        std::optional<ParseResult> chainedRightOperand{ parseExp2(tokens, diag, targetName, ++tp) };

        if (!chainedRightOperand)
        {
            if (!diag.has_value())
            {
                diag = RuntimeError
                {
                    "Parser Error at col " + std::to_string(opPos + 1) + ": Expected right operand after operator '" + opLexeme + "'.",
                    ErrorSeverity::Error,
                    opPos,
                    opLexeme.length()
                };
            }
            return std::nullopt;
        }

        tp = chainedRightOperand->nextTP;

        using Parser::nodes;

        chainedOperator.children = { parentIdx, chainedRightOperand->nodeIdx, -1 };
        int chainedOperatorIdx{ static_cast<int>(nodes.size()) };

        nodes.push_back(chainedOperator);

        return ParseResult{ tp, chainedOperatorIdx };
    }
    else
    {
        diag = RuntimeError
        {
            "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Unexpected trailing token '" + std::string(tokens[tp].lexeme) + "'.",
            ErrorSeverity::Error,
            tokens[tp].charPosition,
            tokens[tp].lexeme.length()
        };

        return std::nullopt;
    }
}

std::optional<ParseResult> parseExp2(const std::vector<Token>& tokens, Diag& diag, OptName& targetName, size_t tp)
{
    using Context::RuntimeError;
    using Context::ErrorSeverity;

    int lOperandIdx{};
    int rOperandIdx{};

    if (tp < tokens.size())
    {
        std::optional<ParseResult> lOperand{ parsePrimary(tokens, diag, targetName, tp) };

        if (!lOperand)
        {
            return std::nullopt;
        }

        tp = lOperand->nextTP;

        if (tp < tokens.size() && tokens[tp].lexeme == "*")
        {
            size_t operatorPos{ tokens[tp].charPosition };

            lOperandIdx = lOperand->nodeIdx;

            std::optional<ParseResult> rOperand{ parsePrimary(tokens, diag, targetName, ++tp) };

            if (!rOperand)
            {
                if (!diag.has_value())
                {
                    diag = RuntimeError
                    {
                        "Parser Error at col " + std::to_string(operatorPos + 1) + ": Expected right operand after '*' operator.",
                        ErrorSeverity::Error,
                        operatorPos,
                        1
                    };
                }
                return std::nullopt;
            }

            tp = rOperand->nextTP;

            rOperandIdx = rOperand->nodeIdx;

            using Parser::nodes;

            int parentIdx{ static_cast<int>(nodes.size()) };
            nodes.push_back({ Node::Exp2, "*", operatorPos });
            nodes.back().children = { lOperandIdx, rOperandIdx, -1 };

            return ParseResult{ tp, parentIdx };
        }

        return ParseResult{ tp, lOperand->nodeIdx };
    }

    return std::nullopt;
}

std::optional<ParseResult> parseExp1(const std::vector<Token>& tokens, Diag& diag, OptName& targetName, size_t tp)
{
    using Context::RuntimeError;
    using Context::ErrorSeverity;

    int lOperandIdx{};
    int rOperandIdx{};

    if (tp < tokens.size())
    {
        std::optional<ParseResult> lOperand{ parseExp2(tokens, diag, targetName, tp) };

        if (!lOperand)
        {
            return std::nullopt;
        }

        tp = lOperand->nextTP;

        if (tp < tokens.size() && (tokens[tp].lexeme == "+" || tokens[tp].lexeme == "-"))
        {
            size_t operatorPos{ tokens[tp].charPosition };
            std::string_view op{ tokens[tp].lexeme };

            lOperandIdx = lOperand->nodeIdx;

            std::optional<ParseResult> rOperand{ parseExp2(tokens, diag, targetName, ++tp) };

            if (!rOperand)
            {
                if (!diag.has_value())
                {
                    diag = RuntimeError
                    {
                        "Parser Error at col " + std::to_string(operatorPos + 1) + ": Expected right operand after '" + std::string(op) + "' operator.",
                        ErrorSeverity::Error,
                        operatorPos,
                        op.length()
                    };
                }
                return std::nullopt;
            }

            tp = rOperand->nextTP;

            rOperandIdx = rOperand->nodeIdx;

            using Parser::nodes;

            int parentIdx{ static_cast<int>(nodes.size()) };
            nodes.push_back({ Node::Exp1, op, operatorPos });
            nodes.back().children = { lOperandIdx, rOperandIdx, -1 };

            if (tp < tokens.size())
            {
                return parseChainedOperator(tokens, diag, targetName, tp, parentIdx);
            }

            return ParseResult{ tp, parentIdx };
        }

        return ParseResult{ tp, lOperand->nodeIdx };
    }

    return std::nullopt;
}

std::optional<ParseResult> parsePrimary(const std::vector<Token>& tokens, Diag& diag, OptName& targetName, size_t tp)
{
    using Context::RuntimeError;
    using Context::ErrorSeverity;

    if (tokens.empty())
    {
        diag = RuntimeError
        {
            "Parser Warning at col 0: Input field is empty.",
            ErrorSeverity::Warning,
            0,
            0
        };

        return std::nullopt;
    }

    static int parserCalls{ 0 };

    using Parser::nodes;

    if (tp >= tokens.size())
    {
        diag = RuntimeError
        {
            "Parser Error at col " + std::to_string(tokens.back().charPosition + 1) + ": Unexpected end of expression. Expected an operand.",
            ErrorSeverity::Error,
            tokens.back().charPosition,
            tokens.back().lexeme.length()
        };

        return std::nullopt;
    }

    const Token* token{ &tokens[tp] };

    if (token->type == Token::Equals)
    {
        diag = RuntimeError
        {
            "Parser Error at col " + std::to_string(token->charPosition + 1) + ": Unexpected assignment operator ('=').",
            ErrorSeverity::Error,
            token->charPosition,
            1
        };

        Parser::nodes.clear();
        return std::nullopt;
    }

    if (token->type == Token::Number)
    {
        if (parserCalls == 0 && tp + 1 < tokens.size() && tokens[tp + 1].type != Token::BiOperator)
        {
            diag = RuntimeError
            {
                "Parser Error at col " + std::to_string(token->charPosition + 1) + ": Standalone numbers must be assigned to a variable.",
                ErrorSeverity::Error,
                token->charPosition,
                token->lexeme.size()
            };

            return std::nullopt;
        }

        int myIdx{ static_cast<int>(nodes.size()) };
        nodes.push_back({ Node::Literal, token->lexeme, token->charPosition });

        if (parserCalls > 0 && tp + 1 < tokens.size() && tokens[tp + 1].type == Token::BiOperator)
        {
            return parseChainedOperator(tokens, diag, targetName, ++tp, myIdx);
        }

        return ParseResult{ tp + 1, myIdx };
    }

    if (token->type == Token::Identifier)
    {
        if (tp == 0 && tp + 2 < tokens.size() && tokens[tp + 1].type == Token::Equals)
        {
            if (tokens[tp + 2].type != Token::Identifier && tokens[tp + 2].type != Token::Number)
            {
                diag = RuntimeError
                {
                    "Parser Error at col " + std::to_string(tokens[tp + 2].charPosition + 1) +
                    ": Expected Identifier or Number after assignment, given '" + std::string(convertTokenTo_string_view(tokens[tp + 2].type)) + "'.",
                    ErrorSeverity::Error,
                    tokens[tp + 2].charPosition,
                    tokens[tp + 2].lexeme.length()
                };

                return std::nullopt;
            }

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
                    "Parser Error at col " + std::to_string(tokens[tp + 1].charPosition + 1) + ": Function '" + std::string(token->lexeme) + "' cannot be empty.",
                    ErrorSeverity::Error,
                    tokens[tp + 1].charPosition,
                    1
                };

                return std::nullopt;
            }

            int parentIdx{ static_cast<int>(nodes.size()) };

            nodes.push_back({ Node::Function, token->lexeme, token->charPosition });

            tp += 2;
            size_t childCount{ 0 };
            int lParenCount{ 1 };
            int rParenCount{ 0 };

            while (tp < tokens.size() && tokens[tp].type != Token::RParen)
            {
                ++parserCalls;
                std::optional<ParseResult> childResult{ parsePrimary(tokens, diag, targetName, tp) };
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
                        "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Argument overflow for function '" + std::string(token->lexeme) + "'.",
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
                            "Parser Error at col " + std::to_string(tokens[tp + 1].charPosition + 1) + ": Misplaced consecutive comma.",
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
                    "Parser Error at col " + std::to_string(tokens[tp - 1].charPosition + 1) + ": Unmatched parenthesis in function call.",
                    ErrorSeverity::Error,
                    tokens[tp - 1].charPosition,
                    tokens[tp - 1].lexeme.length()
                };

                nodes.clear();

                return std::nullopt;
            }

            tp++;

            if (parserCalls == 0 && tp < tokens.size() && tokens[tp].type != Token::BiOperator)
            {
                diag = RuntimeError
                {
                    "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Unexpected trailing token '" + std::string(tokens[tp].lexeme) + "'.",
                    ErrorSeverity::Error,
                    tokens[tp].charPosition,
                    tokens[tp].lexeme.length()
                };

                nodes.clear();

                return std::nullopt;
            }

            else if (parserCalls > 0 && tp < tokens.size() && tokens[tp].type == Token::BiOperator)
            {
                return parseChainedOperator(tokens, diag, targetName, tp, parentIdx);
            }

            else if (parserCalls > 0 && tp < tokens.size() && (tokens[tp].type == Token::Identifier || tokens[tp].type == Token::Number) && tokens[tp - 1].type != Token::Comma)
            {
                diag = RuntimeError
                {
                    "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Missing comma between arguments.",
                    ErrorSeverity::Error,
                    tokens[tp].charPosition,
                    tokens[tp].lexeme.length()
                };

                nodes.clear();

                return std::nullopt;
            }

            return ParseResult{ tp, parentIdx };
        }

        else if (targetName && tokens[tp].type == Token::Number)
        {
            int myIdx = static_cast<int>(nodes.size());
            nodes.push_back({ Node::Literal, token->lexeme, token->charPosition });
            nodes[myIdx].targetName = *targetName;

            return ParseResult{ ++tp, myIdx };
        }

        else
        {
            int myIdx = static_cast<int>(nodes.size());
            nodes.push_back({ Node::Variable, token->lexeme, token->charPosition });

            if (parserCalls > 0 && tp + 1 < tokens.size() && tokens[tp + 1].type == Token::BiOperator)
            {
                return parseChainedOperator(tokens, diag, targetName, ++tp, myIdx);
            }

            return ParseResult{ tp + 1, myIdx };
        }
    }

    if (tp > 0 && tp - 1 < tokens.size() && tokens[tp - 1].type == Token::BiOperator)
    {
        std::string lexemeVal = (tp < tokens.size()) ? std::string(tokens[tp].lexeme) : "<EOF>";
        size_t errCol = (tp < tokens.size()) ? tokens[tp].charPosition : tokens.back().charPosition;
        size_t errLen = (tp < tokens.size()) ? tokens[tp].lexeme.length() : 1;

        diag = RuntimeError
        {
            "Parser Error at col " + std::to_string(errCol + 1) + ": Expected an operand, given '" + lexemeVal + "'.",
            ErrorSeverity::Error,
            errCol,
            errLen
        };

        return std::nullopt;
    }

    diag = RuntimeError
    {
        "Parser Error at col " + std::to_string(tokens[tp].charPosition + 1) + ": Unexpected token '" + std::string(tokens[tp].lexeme) + "'.",
        ErrorSeverity::Error,
        tokens[tp].charPosition,
        tokens[tp].lexeme.length()
    };

    return std::nullopt;
}
