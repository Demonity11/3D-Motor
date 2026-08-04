#ifndef DRAW_UTILS_H
#define DRAW_UTILS_H

#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Context.h"
#include "parser.h"

struct GLFWwindow;

// forward declarations for main.cpp
auto vertexSpec(const std::vector<float>& vertices)														    -> void;
auto updateBufferData(const std::vector<float>& vertices)												    -> void;

// forward declarations for interface.cpp
auto initializeImGui(GLFWwindow* window)																	-> void;
auto setupCustomTheme()																						-> void;
auto menuBar()																								-> void;
auto getUserInput(std::vector<Object>& object)													   	        -> void;
auto getObjectInputFloats(Object& obj)																		-> bool;
void processInput
(
	char inputBuffer[128], 
	const std::vector<FunctionArgs>& funcOverloads, 
	const std::vector<Object>& object, 
	std::optional<Context::RuntimeError>& diag
);
auto showVariables(std::vector<Object>& object)														    	-> void;

auto debugWindow()																							-> void;

auto addToastNotification(const Toast& toast)																-> void;
auto pushErrorStyle(const std::optional<Context::RuntimeError>& diag)										-> void;
auto popErrorStyle(const std::optional<Context::RuntimeError>& diag)										-> void;

auto generateObjectVertices(Object& obj, const std::vector<Object>& object, std::vector<float>& vertexData)	-> int;
void extractAndRegisterObject
(
	const RuntimeValue& evalObj, 
	const std::vector<Object>& object, 
	const std::vector<Node>& nodes, 
	const std::optional<std::string>& targetName
);

void drawObjectLabels
(
	const std::vector<Object>& object,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize
);
void drawAxisLabels
(
	const std::vector<Object>& object,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize
);

#endif
