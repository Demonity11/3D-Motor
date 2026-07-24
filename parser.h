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
		Function,
		Variable,
		Literal
	};

	Node::Type type{};
	std::string_view content{};
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

auto parser(const std::vector<Token>& tokens, size_t tp = 0) -> std::optional<ParseResult>;
auto printNodes(const std::vector<Node>& nodes) -> void;

#endif