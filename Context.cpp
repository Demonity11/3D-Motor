#include "Context.h"

namespace Context
{
	unsigned int VBO{};
	unsigned int VAO{};

	glm::vec3 cameraPos{ 0.0f, 0.5f,  3.0f };
	glm::vec3 cameraTarget{ 0.0f, 0.0f,  0.0f };
	glm::vec3 worldUp{ 0.0f, 1.0f,  0.0f };

	float lastX{ 400.0f };
	float lastY{ 300.0f };

	float yaw{};
	float pitch{};

	float fov{ 45.0f };

	bool isPressingRightClick{ false };
	bool isFirstMouse{ true };
	bool isEnterPressed{ false };
	bool leftClickPressed{ false };

	bool debugWindow{ false };

	std::vector<float> vertexData{};
	// Objects conteiner
	std::vector<Object> object{};

	// store FunctionArgs which has name, type, and expected arguments
	std::vector<FunctionArgs> function
	{
		{"Point",     Object::Point,   {Object::Number, Object::Number, Object::Number} },
		{"Vector",    Object::Vector,  {Object::Point,  Object::Point}				    },
		{"Vector",    Object::Vector,  {Object::Point}								    },
		{"Cross",     Object::Vector,  {Object::Vector, Object::Vector}                 },
		{"Segment",   Object::Segment, {Object::Point,  Object::Point}				    },
		{"Line",	  Object::Line,    {Object::Point,  Object::Point}				    },
		{"Line",      Object::Line,    {Object::Point,  Object::Vector}				    },
		{"Plane",     Object::Plane,   {Object::Point,  Object::Vector}				    },
		{"Plane",     Object::Plane,   {Object::Point,  Object::Point, Object::Point}   },
		{"Intersect", Object::Point,   {Object::Line,   Object::Plane}			        },
		{"Intersect", Object::Point,   {Object::Plane,  Object::Line}			        },
		{"Intersect", Object::Point,   {Object::Line,   Object::Line}			        },
		{"Intersect", Object::Line,    {Object::Plane,  Object::Plane}				    },
		{"Delete",    Object::Null,    {Object::Variable}							    }
	};

	std::vector<Toast> toastNotifications{};

	// store object symbols (default name)
	std::map<Object::Type, char> objectSymbols
	{
		{ Object::Vector,  'u' },
		{ Object::Point,   'A' },
		{ Object::Segment, 'f' },
		{ Object::Plane,   'p' },
		{ Object::Line,    'r' }
	};

	std::map<Object::Type, unsigned int> primitives
	{
		{ Object::Vector,  GL_LINES },
		{ Object::Point,   GL_LINES },
		{ Object::Segment, GL_LINES },
		{ Object::Line,    GL_LINES },
		{ Object::Plane,   GL_TRIANGLES }
	};

	std::map<Object::Type, glm::vec4> defaultColors
	{
		{ Object::Vector,  {0.0f, 0.0f, 0.0f, 1.0f} },
		{ Object::Point,   {0.0f, 0.2f, 0.5f, 1.0f} },
		{ Object::Segment, {0.0f, 0.0f, 0.0f, 1.0f} },
		{ Object::Line,    {0.0f, 0.0f, 0.0f, 1.0f} },
		{ Object::Plane,   {0.0f, 0.0f, 0.0f, 0.2f} }
	};

	std::map<std::string, size_t> symbolTable{};

	std::string inputData{};

	std::vector<std::string> redoBuffer{};
}