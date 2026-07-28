#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <charconv>
#include <vector>
#include <optional>
#include <array>
#include <string_view>

#include "Context.h"
#include "parser.h"

class Object;

auto printRuntimeValue(const RuntimeValue& value)   -> void;
auto convertSVToFloat(std::string_view sv)			-> std::optional<float>;

auto evaluator(const std::vector<Node>& nodes, const std::vector<Object>& object, int nodeIdx = 0)					-> RuntimeValue;

auto evaluatePointFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)		-> RuntimeValue;
auto evaluateVectorFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)	-> RuntimeValue;
auto evaluateCrossFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)		-> RuntimeValue;
auto evaluateSegmentFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)	-> RuntimeValue;
auto evaluateLineFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)		-> RuntimeValue;
auto evaluatePlaneFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)		-> RuntimeValue;
auto evaluateIntersectFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes) -> RuntimeValue;
auto evaluateIntersectFunc(const std::vector<RuntimeValue>& args) -> RuntimeValue;

auto findParentsIDs(const std::vector<Node>& nodes)		-> std::array<int, 3>;
auto deduceTypeByIdentifierName(std::string_view func)  -> Object::Type;

#endif