#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <glm/glm.hpp>

#include "Object.h"
#include "Context.h"
#include "evaluator.h"

auto getStringFunctionType(Object::Type type)																						 -> std::string;
auto getObjectTypeFromString(const std::string& funcName)																			 -> Object::Type;
auto getExpression(const Object& obj, const std::vector<Object>& object)															 -> std::string;
auto getRuntimeValueCompAsString(int id, const std::vector<Object>& objectRef)														 -> std::string;
auto getEquation(const Object& obj)																									 -> std::string;
auto intersectionLinePlane(const Eval::Line& line, const Eval::Plane& plane)														 -> RuntimeValue; // Eval::IPoint or RuntimeError
auto intersectionLineLine(const Eval::Line& lineS, const Eval::Line& lineT)															 -> RuntimeValue; // Eval::IPoint or RuntimeError
auto intersectionPlanePlane(const Eval::Plane& plane1, const Eval::Plane& plane2)													 -> RuntimeValue; // Eval::ILine or RuntimeError
auto recalculateIntersect(Object& obj, const std::vector<Object>& object)															 -> bool;
auto scanForIdenticalObject(Object::Type type, const RuntimeValue& components, const std::vector<Object>& object, int ignoreID = -1) -> bool;
auto compareRuntimeValue(Object::Type type, const RuntimeValue& components1, const RuntimeValue& components2)						 -> bool;
auto searchObjectByID(int id, const std::vector<Object>& objectRef)																	 -> int;
auto searchObjectIndexByName(const std::string& objName, const std::vector<Object>& object)											 -> int;
auto rebuildObjectFromParents(Object& obj, const std::vector<Object>& object)														 -> bool;
void rebuildScene(std::vector<Object>& object, std::vector<float>& vertexData);
auto updateObject(int objIndex, const Object& newObj)																				 -> void;
void purgeObjectAndDependents(int targetID, std::vector<Object>& object);
void deleteObjectByID(int targetID, std::vector<Object>& object, std::vector<float>& vertexData);
auto extractPName(const Object& obj)																								 -> std::string;
auto testInput(const std::string& input)																							 -> std::vector<std::string>;

auto getSelectedObjectID(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, std::vector<Object>& object) -> int;
auto updateSelectedObjectColor(int objIndex, std::vector<Object>& object, std::vector<float>& vertexData)		 -> void;

auto createObject(Object obj, int vCount, const RuntimeValue& comp, const glm::vec4& color, int pCount, const std::array<int, 3>& pIDs = { -1, -1, -1 }) -> size_t;
auto createObject(Object obj, int vCount)																											     -> size_t;

auto duduceRuntimeValueType(const RuntimeValue& value) -> Object::Type;
auto extractPoint(const RuntimeValue& val)			   -> std::optional<glm::vec3>;
auto extractLine(const RuntimeValue& val)			   -> std::optional<Eval::Line>;

void softResetScene();
void resetScene();
bool loadSceneFromFile(const std::string& filename);

bool evaluateDeleteFunc(std::optional<Context::RuntimeError>& diag);

void updateInputData(const Object& updatedObj);

bool projectWorldToScreen
(
	const glm::vec3& worldPos,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize,
	glm::vec2& outScreenPos
);

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);

#endif
