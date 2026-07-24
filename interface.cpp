#include "Window.h"
#include "draw_utils.h"
#include "utilities.h"
#include "objectCoords.h"
#include "Random.h"
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include <sstream>
#include <iostream>

#include "fileManager.h"
#include <filesystem>

struct AutocompleteContext 
{
	bool textWasEdited = false;
	std::string textToInject{};
};

int AutocompleteCallback(ImGuiInputTextCallbackData* data) 
{
	AutocompleteContext* ctx{ (AutocompleteContext*)data->UserData };

	if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) 
	{
		ctx->textWasEdited = true;
	}

	if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways && !ctx->textToInject.empty())
	{
		data->DeleteChars(0, data->BufTextLen);
		data->InsertChars(0, ctx->textToInject.c_str());
		data->BufDirty = true;

		if (data->BufTextLen > 0 && data->Buf[data->BufTextLen - 1] == ')')
		{
			data->CursorPos = data->BufTextLen - 1;
		}

		ctx->textToInject = "";
	}

	return 0;
}

// initializes ImGui context
void initializeImGui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 

	if (io.WantCaptureMouse)

	io.Fonts->AddFontDefault();

	Context::spaceFont = io.Fonts->AddFontFromFileTTF("fonts/Inter_24pt-Bold.ttf", Context::fontSize);

	static_cast<void>(io);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void menuBar()
{
	static bool openNewFilePopup{ false };
	static std::string overwriteFilename{};

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::BeginMenu("Open"))
			{
				// Iterate over all files and subdirectories in the given path
				for (const auto& entry : std::filesystem::directory_iterator("save/"))
				{
					if (ImGui::MenuItem(entry.path().filename().string().c_str()))
					{
						loadSceneFromFile(entry.path().filename().string());
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Save"))
			{
				if (ImGui::MenuItem("New File"))
				{
					openNewFilePopup = true;
				}

				if (ImGui::BeginMenu("Overwrite"))
				{
					for (const auto& entry : std::filesystem::directory_iterator("save/"))
					{
						if (ImGui::MenuItem(entry.path().filename().string().c_str()))
						{
							overwriteFilename = entry.path().filename().string();
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Delete"))
			{
				// Iterate over all files and subdirectories in the given path
				for (const auto& entry : std::filesystem::directory_iterator("save/"))
				{
					if (ImGui::MenuItem(entry.path().filename().string().c_str()))
					{
						removeFile(entry.path().filename().string());
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	if (overwriteFilename.size() > 0)
	{
		writeFile(overwriteFilename, Context::inputData);
		overwriteFilename = "";
	}

	if (openNewFilePopup)
	{
		ImGui::OpenPopup("Create New File");
		openNewFilePopup = false;
	}

	// creates new file
	if (ImGui::BeginPopupModal("Create New File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char inputBuf[20]{};

		bool enterPressed{ ImGui::InputTextWithHint("File name", "Enter file name (e.g.: banana)", inputBuf, sizeof(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue) };

		const std::string filename{ inputBuf };

		if ((ImGui::Button("Save") || enterPressed) && filename.size() > 0 && Context::inputData.size() > 0)
		{
			const std::string filenamePlusExtension{ filename + ".geo" };
			bool foundExistingFile{ false };

			for (const auto& entry : std::filesystem::directory_iterator("save/"))
			{
				if (filenamePlusExtension == entry.path().filename().string())
				{
					std::cerr << "ERROR::FILENAME<" << filename << ">::ALREADY_EXIST\n";
					foundExistingFile = true;
				}
			}
			
			if (!foundExistingFile)
			{
				writeFile(filename, Context::inputData);
			}

			inputBuf[0] = '\0'; 
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			inputBuf[0] = '\0';
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

// captures user input through Dear ImGui interface
void getUserInput(std::vector<Object>& object)
{
	constexpr std::size_t bufferSize{ 128 };
	static char inputBuffer[bufferSize] = "";

	static ImGuiID activePopupID{ 0 };
	static int selectedIndex{ -1 };
	static bool showDropdown{ false };

	static AutocompleteContext context;
	static bool focusNextFrame{ false };

	ImGuiInputTextFlags inputFlags
	{ 
		ImGuiInputTextFlags_EnterReturnsTrue | 
		ImGuiInputTextFlags_CallbackEdit |
		ImGuiInputTextFlags_CallbackAlways
	};

	ImGuiID inputID{ ImGui::GetID("Input") };

	if (focusNextFrame)
	{
		ImGui::SetKeyboardFocusHere(0);
		focusNextFrame = false;
	}

	bool isEnterPressed{ ImGui::InputTextWithHint("Input", "input", inputBuffer, IM_COUNTOF(inputBuffer), inputFlags, AutocompleteCallback, &context) };

	bool isInputActive{ ImGui::IsItemActive() };
	bool isInputFocused{ ImGui::IsItemFocused() };

	if (isInputActive || isInputFocused) {
		activePopupID = inputID;
	}

	if (context.textWasEdited && activePopupID == inputID) {
		showDropdown = true;
	}

	std::string currentText(inputBuffer);
	std::vector<FunctionArgs> matches{};

	if (showDropdown && !currentText.empty() && activePopupID == inputID)
	{
		for (const auto& func : Context::function)
		{
			if (func.name.rfind(currentText, 0) == 0 && func.name != currentText)
				matches.push_back(func);
		}
	}

	if (!matches.empty() && activePopupID == inputID && showDropdown)
	{
		ImVec2 inputPosMin{ ImGui::GetItemRectMin() };
		ImVec2 inputPosMax{ ImGui::GetItemRectMax() };

		ImGui::SetNextWindowPos(ImVec2(inputPosMin.x, inputPosMax.y));
		ImGui::SetNextWindowSize(ImVec2(inputPosMax.x - inputPosMin.x, 0));

		ImGuiWindowFlags flags
		{
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_ChildWindow
		};

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
		
		if (ImGui::Begin("##AutocompletePopup", nullptr, flags))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) { selectedIndex++; }
			if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) { selectedIndex--; }

			if (selectedIndex < 0) selectedIndex = static_cast<int>(matches.size() - 1);
			if (selectedIndex >= static_cast<int>(matches.size())) selectedIndex = 0;

			for (int i{ 0 }; i < static_cast<int>(matches.size()); ++i) 
			{
				const bool isSelected{ (i == selectedIndex) };

				const auto& match{ matches[i] };

				std::string func{ match.name + "(" };

				if (match.name == "Point")
					func += ")";

				for (std::size_t i{ 0 }; i < match.expectedArgs.size(); ++i)
				{
					const auto arg{ getStringFunctionType(match.expectedArgs[i]) };

					if (i < match.expectedArgs.size() - 1)
						func += arg + ", ";
					else
						func += arg + ")";
				}

				if (ImGui::Selectable(func.c_str(), isSelected))
				{
					context.textToInject = matches[i].name + "()";
					context.textWasEdited = false;
					showDropdown = false;
					selectedIndex = -1;
					focusNextFrame = true;
				}

				if (isSelected && ImGui::IsKeyPressed(ImGuiKey_Tab))
				{
					context.textToInject = matches[i].name + "()";
					context.textWasEdited = false;
					showDropdown = false;
					selectedIndex = -1;
					focusNextFrame = true;
				}
			}
		}

		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseClicked(0)) 
		{
			showDropdown = false;
			selectedIndex = -1;
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	if (isEnterPressed) 
	{
		showDropdown = false;
		selectedIndex = -1;

		processInput(inputBuffer, Context::function, object);

		ImGui::SetKeyboardFocusHere(-1);
	}

	ImGui::SeparatorText("Variables");
}

void processInput(char inputBuffer[128], const std::vector<FunctionArgs>& function, const std::vector<Object>& object)
{
	//auto ss{ std::stringstream(inputBuffer) };
	std::string inputText{ inputBuffer };

	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(3,3,3)\nSegment(A,B)\nVector(A)\nLine(A,u)\nPlane(A,u)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nPlane(A,B,C)\nVector(A,B)\nVector(B,C)\nPoint(3,-2,-3)\nLine(A,D)\nCross(u,v)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nVector(A,B)\nVector(A,C)\nCross(u,v)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nPlane(A,B,C)\nPoint(-3,2,1)\nPoint(4,-2,3)\nLine(D,E)\nIntersect(r,p)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nPlane(A,B,C)\nPoint(-3,2,1)\nPoint(4,-2,3)\nPoint(2,1,-3)\nPlane(D,E,F)\nIntersect(p,q)\nVector(A,B)\nVector(D,E)\nCross(u,v)\n") };
	//static auto inputArray{ testInput(
	//	"Point(1,1,1)\n"
	//	"Point(2,2,2)\n"
	//	"Point(3,3,3)\n"
	//	"Plane(Point(-2,1,-3), Vector(B,C))\n"
	//	"Cross(Vector(A,B), Vector(Point(-2,-1,3), Point(-3,1,-2)))\n"
	//	"Line(A,B)\n"
	//	"Intersect(r,p)\n"
	//) };

	//std::cout << openFile("aaa.geo");

	static auto inputArray{ readFile("aaa.geo") };
	std::cout << inputArray.size() << "\n";

	//static auto inputArray{ testInput("") };

	// types input faster for testing
	if (!inputArray.empty())
	{
		inputText = inputArray[0];
		inputArray.erase(inputArray.begin());
	}

	tokenizer(inputText);

	printTokens(Lexer::tokens);

	parser(Lexer::tokens);

	printNodes(Parser::nodes);

	RuntimeValue evalObj{ evaluator(Parser::nodes, object) };

	printRuntimeValue(evalObj);

	if (std::holds_alternative<Context::RuntimeError>(evalObj))
	{
		Lexer::tokens.clear();
		Parser::nodes.clear();

		return;
	}

	extractAndRegisterObject(evalObj, object, Parser::nodes);
	Lexer::tokens.clear();
	Parser::nodes.clear();

	Context::inputData += inputText + "\n";
	inputBuffer[0] = '\0';
}

void showVariables(std::vector<Object>& object)
{
	bool isSelectionChanged{ false };

	if (Context::selectedObjID != -1)
	{
		if (Context::prevSelectedObjID != -1 && Context::selectedObjID != Context::prevSelectedObjID)
		{
			isSelectionChanged = true;
		}

		else
		{
			Context::prevSelectedObjID = Context::selectedObjID;
		}
	}

	for (size_t i{ 8 }; i < object.size(); ++i)
	{
		Object& obj{ object[i] };

		std::string headerText{ obj.getName() + ": " + getExpression(obj, object) + "###" + obj.getName() };

		const int currentID{ obj.getID() };
		const int currentIndex{ static_cast<int>(i) };

		if (Context::selectedObjID == -1 && Context::prevSelectedObjID != -1)
		{
			if (currentID == Context::prevSelectedObjID)
			{
				ImGui::SetNextItemOpen(false);
				Context::prevSelectedObjID = Context::selectedObjID;
				obj.setSelected(false);
				updateSelectedObjectColor(currentIndex, object, Context::vertexData);
			}
		}

		if (isSelectionChanged && Context::prevSelectedObjID == currentID)
		{
			ImGui::SetNextItemOpen(false);
			isSelectionChanged = false;
			Context::prevSelectedObjID = Context::selectedObjID;

			if (obj.isSelected())
			{
				obj.setSelected(false);
				updateSelectedObjectColor(currentIndex, object, Context::vertexData);
			}
		}

		if (currentID == Context::selectedObjID)
		{	

			if (!obj.isSelected())
			{
				ImGui::SetNextItemOpen(true);
				obj.setSelected(true);
				updateSelectedObjectColor(currentIndex, object, Context::vertexData);
			}
		}

		if (ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_None))
		{
			static ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_None;

			if (obj.getType() == Object::Plane || obj.getType() == Object::Line) 
				ImGui::Text(getEquation(obj).c_str());

			bool valuesChanged{ getObjectInputFloats(obj) };

			ImGui::ColorEdit4((obj.getName() + "::Color").c_str(), obj.getColorPointer(), ImGuiColorEditFlags_Float | colorFlags);

			bool colorChanged{ ImGui::IsItemDeactivatedAfterEdit() };

			if (valuesChanged || colorChanged) // saves the changes
				updateObject(static_cast<int>(i), obj, object, Context::vertexData);

			std::string deleteText{ "Delete###" + std::to_string(obj.getID()) };

			if (ImGui::Button(deleteText.c_str()))
			{
				deleteObject(static_cast<int>(i), object, Context::vertexData);
				Context::prevSelectedObjID = -1;
				Context::selectedObjID = -1;
				break;
			}
		}
	}
}

bool getObjectInputFloats(Object& obj)
{
	RuntimeValue& comp{ obj.getComponents() };

	ImGuiInputFlags textFlags{};

	if (!obj.isMutable()) textFlags |= ImGuiInputTextFlags_ReadOnly;

	bool isDeactivated{ false };

	auto checkInput = [&](const std::string& label, float* data) {
		ImGui::InputFloat3((obj.getName() + label).c_str(), data, "%.2f", textFlags);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			isDeactivated = true;
		}
		};

	std::visit(overloaded
		{
		[](float f)
		{},
		[&](glm::vec3& point)
		{
			checkInput("::Point", &point[0]);
		},
		[&](Eval::IPoint& iPoint)
		{
			checkInput("::Point", &iPoint.point[0]);
		},
		[&](Eval::Vector& vector)
		{
			checkInput("::Origin", &vector.origin[0]);
			checkInput("::Head", &vector.head[0]);
		},
		[&](Eval::Segment& segment)
		{
			checkInput("::A", &segment.A[0]);
			checkInput("::B", &segment.B[0]);
		},
		[&](Eval::Line& line)
		{
			checkInput("::Point", &line.point[0]);
			checkInput("::DVecOrigin", &line.dVecOrigin[0]);
			checkInput("::DVecHead", &line.dVecHead[0]);
		},
		[&](Eval::ILine& iLine)
		{
			checkInput("::Point", &iLine.line.point[0]);
			checkInput("::DVecOrigin", &iLine.line.dVecOrigin[0]);
			checkInput("::DVecHead", &iLine.line.dVecHead[0]);
		},
		[&](Eval::Plane& plane)
		{
			checkInput("::Point", &plane.point[0]);
			checkInput("::NormalOrigin", &plane.normalOrigin[0]);
			checkInput("::NormalHead", &plane.normalHead[0]);
		},
		[](Context::RuntimeError error)
		{
			std::cerr << error.message << '\n';
		}

		}, comp);

	return isDeactivated;
}

int generateObjectVertices(Object& obj, const std::vector<Object>& object, std::vector<float>& vertexData)
{
	//using Context::object;

	constexpr float scale{ 0.1f };

	// object data
	Object::Type type{ obj.getType() };
	const std::array<int, 3>& pIDs{ obj.getParentIDs() };
	//const std::array<int, 3>& pCompIndex{ obj.getpCompIndex() };
	const glm::vec4& color{ obj.getColor() };

	int vCount{ 0 };

	// intersection
	if (type == Object::Point && pIDs[0] >= 0)
	{
		Eval::IPoint intersection{ std::get<Eval::IPoint>(obj.getComponents()) };

		if (scanForIdenticalObject(type, intersection, object, obj.getID()))
		{
			std::cout << intersection.point << "\n";
			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
			return -1;
		}

		obj.setMutable(false);
		obj.setColor({ 0.7f, 0.3f, 0.0f, 1.0f });

		intersection.point *= scale;
		constexpr float radius{ 0.005f };

		vCount = getSphereVertices(intersection.point, color, radius, vertexData);
	}

	else if (type == Object::Point)
	{
		glm::vec3 point{ std::get<glm::vec3>(obj.getComponents()) };

		point *= scale;

		constexpr float radius{ 0.005f };

		// getSphereVertices create 120960 new floats => 120960 / 7 = 17280 vertices, where 7 = number of components
		vCount = getSphereVertices(point, color, radius, vertexData);
	}

	else if (type == Object::Vector)
	{
		//std::array<glm::vec3, 2> vector{ assemblyVector(obj, object) };
		Eval::Vector vector{ std::get<Eval::Vector>(obj.getComponents()) };

		glm::vec3 vecOrigin{ vector.origin };
		glm::vec3 vecHead{ vector.head };

		constexpr float epsilon{ 0.001f };
		if (glm::length(vecHead - vecOrigin) < epsilon)
			return -1;

		vecOrigin *= scale;
		vecHead *= scale;

		constexpr float radius{ 0.0015f };
		constexpr float coneRadius{ radius * 4.0f };

		const glm::vec3 direction{ glm::normalize(vecHead - vecOrigin) };
		const float cilinderLength{ glm::length(vecHead - vecOrigin) };
		constexpr float coneHeight{ 0.025f };

		float actualCilinderLength{ glm::max(0.0f, cilinderLength - coneHeight) };

		glm::vec3 newVecHead{ vecOrigin + actualCilinderLength * direction };

		int vCountCilinder{};
		int vCountCone{};

		if (vector.pTypes[1] == ObjectType::Vector)
			obj.setMutable(false);

		// getCilinderVertices creates 144 new vertices 
		vCountCilinder = getCilinderVertices(vecOrigin, newVecHead, color, radius, vertexData);
		vCountCone = getConeVertices(direction, vecHead, color, coneRadius, coneHeight, vertexData);

		vCount = vCountCilinder + vCountCone;
	}

	else if (type == Object::Segment)
	{
		Eval::Segment segment{ std::get<Eval::Segment>(obj.getComponents()) };

		glm::vec3& pointA{ segment.A };
		glm::vec3& pointB{ segment.B };

		pointA *= scale;
		pointB *= scale;

		constexpr float radius{ 0.0015f };

		vCount = getCilinderVertices(pointA, pointB, color, radius, vertexData);
	}

	else if (type == Object::Line && std::holds_alternative<Eval::ILine>(obj.getComponents()))
	{
		Eval::ILine intersection{ std::get<Eval::ILine>(obj.getComponents()) };

		constexpr float epsilon{ 0.001f };
		if (glm::length(intersection.line.dVecHead -intersection.line.dVecOrigin) < epsilon)
		{
			std::cerr << "Intersection doesn't exist.\n";
			return -1;
		}

		if (scanForIdenticalObject(type, intersection, object, obj.getID()))
		{
			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
			return -1;
		}

		obj.setMutable(false);
		obj.setColor({ 0.7f, 0.3f, 0.0f, 1.0f });
		obj.setComponents(intersection);

		intersection.line.point *= scale;
		intersection.line.dVecOrigin *= scale;
		intersection.line.dVecHead *= scale;
		constexpr float radius{ 0.0015f };
		
		vCount = getLineVertices(intersection.line.point, { 0.0f, 0.0f, 0.0f }, intersection.line.dVecHead - intersection.line.dVecOrigin, color, radius, vertexData);
	}

	else if (type == Object::Line)
	{
		Eval::Line line{ std::get<Eval::Line>(obj.getComponents()) };

		line.point *= scale;
		line.dVecOrigin *= scale;
		line.dVecHead *= scale;
		constexpr float radius{ 0.0015f };

		vCount = getLineVertices(line.point, line.dVecOrigin, line.dVecHead, color, radius, vertexData);
	}

	else if (type == Object::Plane)
	{
		Eval::Plane plane{ std::get<Eval::Plane>(obj.getComponents()) };

		plane.point *= scale;
		plane.normalOrigin *= scale;
		plane.normalHead *= scale;

		printRuntimeValue(plane);

		// getPlaneVertices create 6 new vertices
		vCount = getPlaneVertices(plane.normalOrigin, plane.normalHead, plane.point, color, vertexData);
	}

	return vCount;
}

void drawObjectLabels
(
	const std::vector<Object>& object,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize
)
{
	ImDrawList* drawList{ ImGui::GetBackgroundDrawList() };

	constexpr float scale{ 0.1f };

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		const Object& obj{ object[idx] };
		Object::Type type{ obj.getType() };

		if (type == Object::Line || type == Object::Plane)
			continue;

		glm::vec3 targetWorldPos{};
		const RuntimeValue& comp{ obj.getComponents() };

		if (type == Object::Vector && std::holds_alternative<Eval::Vector>(comp))
		{
			targetWorldPos = std::get<Eval::Vector>(comp).head;

			targetWorldPos *= scale;
			targetWorldPos += glm::vec3{ 0.0f, 0.03f, 0.0f };
		}

		else if (type == Object::Point && std::holds_alternative<glm::vec3>(comp))
		{
			targetWorldPos = std::get<glm::vec3>(comp);
			targetWorldPos *= scale;
			targetWorldPos += glm::vec3{ 0.0f, 0.015f, 0.0f };
		}

		else if (type == Object::Point && std::holds_alternative<Eval::IPoint>(comp))
		{
			targetWorldPos = std::get<Eval::IPoint>(comp).point;
			targetWorldPos *= scale;
			targetWorldPos += glm::vec3{ 0.0f, 0.015f, 0.0f };
		}

		else if (type == Object::Segment && std::holds_alternative<Eval::Segment>(comp))
		{
			const Eval::Segment& seg{ std::get<Eval::Segment>(comp) };

			targetWorldPos = (seg.A + seg.B) * 0.5f;
			targetWorldPos *= scale;
		}

		glm::vec2 screenPos{};

		if (projectWorldToScreen(targetWorldPos, viewMatrix, projectionMatrix, modelMatrix, viewportPos, viewportSize, screenPos))
		{
			std::string label{ obj.getName() };

			float textWidth{ Context::spaceFont->CalcTextSizeA(Context::fontSize, FLT_MAX, 0.0f, label.c_str()).x };
			ImVec2 adjustedPos{ screenPos.x - (textWidth * 0.5f), screenPos.y };

			drawList->AddText(Context::spaceFont, Context::fontSize, ImVec2{ adjustedPos.x + 1.0f, adjustedPos.y + 1.0f }, ImColor{ 0, 0, 0, 255 }, label.c_str());
			drawList->AddText(Context::spaceFont, Context::fontSize, adjustedPos, ImColor{ 0, 80, 255, 255 }, label.c_str());
		}
	}
}

void drawAxisLabels
(
	const std::vector<Object>& object,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize
)
{
	// ring start vertices
	std::vector ringVertices
	{
		-1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,

		 0.0f, -1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		  
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f,  1.0f,
	};

	std::vector<ImColor> axisColors
	{
		{ 255, 0, 0, 255 },
		{ 0, 255, 0, 255 },
		{ 0, 0, 255, 255 }
	};

	ImDrawList* drawList{ ImGui::GetBackgroundDrawList() };

	constexpr float stride{ 0.1f };
	constexpr int ringCount{ 21 };
	constexpr float axisSpacing{ -0.0085f };

	bool isZeroDrawn{ false };
	bool isColorWhite{ true };

	for (size_t ringStart{ 0 }, c{ 0 }; ringStart < ringVertices.size(); ringStart += 6, ++c)
	{
		glm::vec3 ringPos{ ringVertices[ringStart], ringVertices[ringStart + 1], ringVertices[ringStart + 2] };
		glm::vec3 direction{ glm::normalize(glm::vec3
		(
			ringVertices[ringStart + 3] - ringVertices[ringStart],
			ringVertices[ringStart + 4] - ringVertices[ringStart + 1],
			ringVertices[ringStart + 5] - ringVertices[ringStart + 2]
		)) };

		if (ringStart == 0) ringPos  += glm::vec3{ 0.0f, axisSpacing, axisSpacing };
		if (ringStart == 6) ringPos  += glm::vec3{ axisSpacing, 0.0f, axisSpacing };
		if (ringStart == 12) ringPos += glm::vec3{ axisSpacing, axisSpacing, 0.0f };

		ImColor color{};

		for (int ring{ 0 }, count{ -10 }; ring < ringCount; ++ring, ++count)
		{
			if (count == 0 && isZeroDrawn)
			{
				ringPos += direction * stride;
				continue;
			}

			glm::vec2 screenPos{};

			if (projectWorldToScreen(ringPos, viewMatrix, projectionMatrix, modelMatrix, viewportPos, viewportSize, screenPos))
			{
				if (count == 0 && isColorWhite) 
				{
					isZeroDrawn = true;
					isColorWhite = false;
					color = ImColor{ 255, 255, 255, 255 };
				}
				else
				{
					color = axisColors[c];
				}

				std::string label{ std::to_string(count) };

				float textWidth{ Context::spaceFont->CalcTextSizeA(Context::fontSize, FLT_MAX, 0.0f, label.c_str()).x };
				ImVec2 adjustedPos{ screenPos.x - (textWidth * 0.5f), screenPos.y };

				drawList->AddText(Context::spaceFont, Context::fontSize, ImVec2{ adjustedPos.x + 1.0f, adjustedPos.y + 1.0f }, ImColor{ 0, 0, 0, 255 }, label.c_str());
				drawList->AddText(Context::spaceFont, Context::fontSize, adjustedPos, color, label.c_str());
			}

			ringPos += direction * stride;
		}
	}
}

void extractAndRegisterObject(const RuntimeValue& evalObj, const std::vector<Object>& object, const std::vector<Node>& nodes)
{
	Object::Type type{ duduceRuntimeValueType(evalObj) };

	std::array<int, 3> pIDs{ findParentsIDs(nodes) };

	if (scanForIdenticalObject(type, evalObj, object))
	{
		std::cerr << "Object already exist.\n";
		return;
	}

	const std::string objName{ Context::objectSymbols[type]++ };
	unsigned int primitive{ Context::primitives[type] };
	glm::vec4 color{ Context::defaultColors[type] };

	int pCount{ 0 };
	for (size_t i{ 0 }; i < std::size(pIDs); ++i)
	{
		if (pIDs[i] != -1 && pIDs[i] != Context::componentLiteral)
			++pCount;
	}

	glm::vec4 finalColor{ color };

	// plane random color
	if (type == Object::Plane)
	{
		float r{}, g{}, b{};

		do {
			r = static_cast<float>(Random::get(0, 10)) * 0.1f;
			g = static_cast<float>(Random::get(0, 10)) * 0.1f;
			b = static_cast<float>(Random::get(0, 10)) * 0.1f;
		} while (r < 0.2f && g > 0.4f && b > 0.7f);

		finalColor = { r, g, b, 0.2f };
	}

	Object obj{ objName, type, Context::primitives[type], evalObj, finalColor, pIDs, pCount };
	int vCount{ generateObjectVertices(obj, object, Context::vertexData) };

	if (vCount == -1)
	{
		std::cerr << "ERROR::FAILED_TO_GENERATE_VERTICES\n";
		return;
	}

	size_t objIdx{ createObject(std::move(obj), vCount) };

	Context::symbolTable[objName] = objIdx;

	updateBufferData(Context::vertexData);

	for (const auto& obj : Context::object)
	{
		obj.printObject();
	}
}