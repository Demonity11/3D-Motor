#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "imgui/imgui.h"
#include "Object.h"

struct ImFont;

struct FunctionArgs
{
	std::string name{};
	Object::Type type{};
	std::vector<Object::Type> expectedArgs{};
};

struct Toast
{
	std::string title{};
	std::string message{};
	ImColor severity{};

	float duration{};
	float timeRemaining{};
};

namespace Context
{
	extern unsigned int VBO;
	extern unsigned int VAO;

	extern glm::vec3 cameraPos;
	extern glm::vec3 cameraTarget;
	extern glm::vec3 worldUp;

	extern float lastX;
	extern float lastY;

	extern float yaw;
	extern float pitch;

	extern float fov;

	extern bool isPressingRightClick;
	extern bool isFirstMouse;
	extern bool isEnterPressed;
	extern bool leftClickPressed;

	extern bool debugWindow;

	extern std::vector<float> vertexData;
	// Objects conteiner
	extern std::vector<Object> object;
	// store FunctionArgs which has name, type, and expected arguments
	extern std::vector<FunctionArgs> function;
	// store object symbols (default name)
	extern std::map<Object::Type, char> objectSymbols;

	extern std::map<Object::Type, unsigned int> primitives;

	extern std::map<Object::Type, glm::vec4> defaultColors;
	// key: object name - value: object index
	extern std::map<std::string, size_t> symbolTable;

	extern std::vector<Toast> toastNotifications;

	// parent ID when the object has literal components
	inline constexpr int componentLiteral{ -2 };

	inline ImFont* spaceFont{ nullptr };

	inline constexpr float fontSize{ 16.0f };

	inline int prevSelectedObjID{ -1 };
	inline int selectedObjID{ -1 };

	inline int globalObjectIDCounter{ 0 };
	inline float defaultToastDuration{ 5.0f };

	extern std::string inputData;
	extern std::vector<std::string> redoBuffer;
}

#endif
