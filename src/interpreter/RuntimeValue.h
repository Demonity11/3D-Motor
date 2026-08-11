#ifndef RUNTIME_VALUE_H
#define RUNTIME_VALUE_H

#include <variant>
#include <string>
#include <glm/glm.hpp>
#include "scene/ObjectType.h"
#include "lexer.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace Eval
{
	struct Vector
	{
		glm::vec3 origin{};
		glm::vec3 head{};

		std::array<ObjectType, 2> pTypes{ ObjectType::Null, ObjectType::Null };
	};

	struct Segment
	{
		glm::vec3 A{};
		glm::vec3 B{};

		std::array<ObjectType, 2> pTypes{ ObjectType::Null, ObjectType::Null };
	};

	struct Line
	{
		glm::vec3 point{};
		glm::vec3 dVecOrigin{};
		glm::vec3 dVecHead{};

		std::array<ObjectType, 2> pTypes{ ObjectType::Null, ObjectType::Null };
	};

	struct Plane
	{
		glm::vec3 point{};
		glm::vec3 normalOrigin{};
		glm::vec3 normalHead{};

		std::array<ObjectType, 3> pTypes{ ObjectType::Null, ObjectType::Null, ObjectType::Null };
	};

	// Intersect Point
	struct IPoint
	{
		glm::vec3 point{};

		std::array<ObjectType, 2> pTypes{ ObjectType::Null, ObjectType::Null };
	};

	// Intersect Line
	struct ILine
	{
		Eval::Line line{};

		std::array<ObjectType, 2> pTypes{ ObjectType::Null, ObjectType::Null };
	};
}

using RuntimeValue = std::variant
<
	float, 
	glm::vec3, 
	Eval::Vector, 
	Eval::Segment, 
	Eval::Line, 
	Eval::Plane, 
	Eval::IPoint,
	Eval::ILine,
	Context::RuntimeError
>;

#endif