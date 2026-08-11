#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <array>
#include <cstdint>
#include <utility>
#include <glm/glm.hpp>
#include <iostream>

#include "ObjectType.h"
#include "RuntimeValue.h"

class Object
{
public:
	using Type = ObjectType;

	static constexpr Type Null = ObjectType::Null;
	static constexpr Type Point = ObjectType::Point;
	static constexpr Type Vector = ObjectType::Vector;
	static constexpr Type Segment = ObjectType::Segment;
	static constexpr Type Line = ObjectType::Line;
	static constexpr Type Plane = ObjectType::Plane;
	static constexpr Type Variable = ObjectType::Variable;
	static constexpr Type Number = ObjectType::Number;

	Object
	(
		std::string name,
		Type type,
		unsigned int primitive,
		const RuntimeValue& components,
		const glm::vec4& color,
		const std::array<int, 3>& pIDs = { -1, -1, -1 },
		int pCount = 0
	)
		: m_name{ name }
		, m_type{ type }
		, m_primitive{ primitive }
		, m_components{ components }
		, m_color{ color }
		, m_parentIDs{ pIDs }
		, m_parentCount{ pCount }
	{
	}

	Object(std::string name, Type type, const RuntimeValue& components)
		: m_name { name }
		, m_type { type }
		, m_components { components }
	{ 
	}

	Object()
		: m_id{ -1 }
	{ }

	void printObject() const
	{
		std::cout << "NAME: " << this->getName() << "\n";
		std::cout << "ID: " << this->getID() << "\n";
		std::cout << "MUTABLE: " << std::boolalpha << this->isMutable() << "\n";
	}

	int						  getID() const          { return m_id; }
	const std::string&		  getName() const        { return m_name; }
	Type				      getType() const        { return m_type; }
	unsigned int		      getPrimitive() const   { return m_primitive; }
	int				          getOffset() const      { return m_offset; }
	int					      getVertexCount() const { return m_vertexCount; }
	const RuntimeValue&		  getComponents() const	 { return m_components; }
	const glm::vec4&          getColor() const		 { return m_color; }
	const std::array<int, 3>& getParentIDs() const   { return m_parentIDs; }
	int					      getParentCount() const { return m_parentCount; }

	void setID(int id)										 { m_id = id; }
	void setName(const std::string& name)					 { m_name = name; }
	void setOffset(int offset)								 { m_offset = offset; }
	void setVertexCount(int vertexCount)					 { m_vertexCount = vertexCount; }
	void setComponents(const RuntimeValue& components)		 { m_components = components; }
	void setColor(const glm::vec4& color)					 { m_color = color; }
	void setParentIDs(const std::array<int, 3>& parentIDs)   { m_parentIDs = parentIDs; }
	void setParentCount(uint8_t parentCount)				 { m_parentCount = parentCount; }

	float* getColorPointer()	  { return &m_color[0]; }
	RuntimeValue& getComponents() { return m_components; }

	bool isMutable() const		 { return m_isMutable; }
	bool isSelected() const		 { return m_isSelected; }
	bool isVisible() const		 { return m_isVisible; }

	void setMutable(bool isMutable)   { m_isMutable = isMutable; }
	void setSelected(bool isSelected) { m_isSelected = isSelected; }
	void setVisible(bool isVisible)   { m_isVisible = isVisible; }

private:
	int m_id{};
	std::string m_name{};
	Type m_type{};
	unsigned int m_primitive{};

	int m_offset{};
	int m_vertexCount{};

	RuntimeValue m_components{};
	glm::vec4 m_color{};

	std::array<int, 3> m_parentIDs{ -1, -1, -1 }; // default values to a object that has no parent
	int m_parentCount{}; // max parent count. for example: a vector can have up to two parents

	bool m_isMutable{ true };
	bool m_isSelected{ false };
	bool m_isVisible{ true };
};

#endif

