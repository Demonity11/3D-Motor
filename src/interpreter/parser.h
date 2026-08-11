#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <array>
#include <string_view>
#include <string>
#include <optional>

#include "lexer.h"

struct Node
{
	enum Type
	{
		// primary
		Function,
		Variable,
		Literal,
		
		// expressions -> Exp1 (Exp = Expression, 1 = level of precedence, higher precedence levels are evaluated first)
		Exp1,
		Exp2
	};

	Node::Type type{};
	std::string_view content{};
	size_t charPosition{};
	std::optional<std::string> targetName{};
	std::array<int, 3> children{ -1, -1, -1 };
};

struct ParseResult 
{
	size_t nextTP{};
	int nodeIdx{};
};

namespace Parser
{
	extern std::vector<Node> nodes;
}

// aliases
using Diag    = std::optional<Context::RuntimeError>;
using OptName = std::optional<std::string>;

constexpr auto convertNodeTo_string_view(Node::Type type)					   -> std::string_view;

auto parsePrimary(const std::vector<Token>& tokens, Diag& diag, OptName& targetName, size_t tp = 0)	   -> std::optional<ParseResult>;
auto parseExp2(const std::vector<Token>& tokens, Diag& diag, OptName& targetName, size_t tp = 0)	   -> std::optional<ParseResult>;
auto parseExp1(const std::vector<Token>& tokens, Diag& diag, OptName& targetName, size_t tp = 0)	   -> std::optional<ParseResult>;

auto parseChainedOperator(const std::vector<Token>& tokens, Diag& diag, OptName& targetName, size_t tp, int parentIdx) -> std::optional<ParseResult>;

auto printNodes(const std::vector<Node>& nodes)																   -> void;

#endif