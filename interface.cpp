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

#include "imgui/imgui_internal.h"

#include "fileManager.h"
#include <filesystem>
#include <format>

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

void setupCustomTheme()
{
	ImGuiStyle& style{ ImGui::GetStyle() };

	style.WindowRounding = 8.0f;
	style.FrameRounding = 5.0f;
	style.PopupRounding = 6.0f;
	style.ScrollbarRounding = 6.0f;
	style.GrabRounding = 4.0f;
	style.TabRounding = 5.0f;

	style.WindowPadding = ImVec2(10.0f, 10.0f);
	style.FramePadding = ImVec2(8.0f, 5.0f);
	style.ItemSpacing = ImVec2(8.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
	style.IndentSpacing = 20.0f;

	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.PopupBorderSize = 1.0f;

	ImVec4* colors = style.Colors;

	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.12f, 0.14f, 0.94f); // #1C1E24
	colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.15f, 0.18f, 0.60f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.15f, 0.18f, 0.98f);

	colors[ImGuiCol_Border] = ImVec4(0.25f, 0.27f, 0.32f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.32f, 0.40f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.27f, 0.34f, 1.00f);

	colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.27f, 0.33f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.32f, 0.40f, 1.00f);

	colors[ImGuiCol_Button] = ImVec4(0.23f, 0.46f, 0.81f, 0.80f); // #3B75CE
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.54f, 0.92f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.19f, 0.40f, 0.72f, 1.00f);

	colors[ImGuiCol_CheckMark] = ImVec4(0.38f, 0.72f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.38f, 0.72f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.23f, 0.56f, 0.88f, 1.00f);

	// Text Colors
	colors[ImGuiCol_Text] = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.53f, 0.60f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.23f, 0.46f, 0.81f, 0.35f);
}

void menuBar()
{
	static bool openNewFilePopup{ false };
	static bool overwriteFilePopup{ false };
	static bool deleteFilePopup{ false };

	static std::string overwriteFilename{};
	static std::string deleteFilename{};

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::MenuItem("Clear Scene"))
			{
				if (Context::globalObjectIDCounter > 0)
				{
					Toast toast
					{
						"Scene Cleared",
						std::to_string(static_cast<int>(Context::object.size()) - 8) + " objects deleted.",
						ImColor{ 255, 255, 0, 255 },
						Context::defaultToastDuration,
						Context::defaultToastDuration
					};

					resetScene();
					addToastNotification(toast);
				}
				else
				{
					Toast toast
					{
						"Scene Not Cleared",
						"The scene was not modified.",
						ImColor{ 0, 0, 255, 255 },
						Context::defaultToastDuration,
						Context::defaultToastDuration
					};
					addToastNotification(toast);
				}
			}

			if (ImGui::BeginMenu("Open"))
			{
				// Iterate over all files and subdirectories in the given path
				for (const auto& entry : std::filesystem::directory_iterator("save/"))
				{
					if (ImGui::MenuItem(entry.path().filename().string().c_str()))
					{
						if (!loadSceneFromFile(entry.path().filename().string()))
						{
							Toast toast
							{
								"Scene Not Loaded",
								"File '" + entry.path().filename().string() + "' was not loaded successfully.",
								ImColor{ 255, 0, 0, 255 },
								Context::defaultToastDuration,
								Context::defaultToastDuration
							};

							addToastNotification(toast);
						}
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
							overwriteFilePopup = true;
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
						deleteFilename = entry.path().filename().string();
						deleteFilePopup = true;
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Undo"))
		{
			undo();
		}

		if (ImGui::MenuItem("Redo"))
		{
			redo();
		}

		if (ImGui::MenuItem("Debug"))
		{
			Context::debugWindow ^= 1;
		}

		ImGui::EndMenuBar();
	}

	if (deleteFilePopup)
	{
		deleteFilePopup = false;
		ImGui::OpenPopup("Delete File");
	}

	if (ImGui::BeginPopupModal("Delete File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure you want to delete '%s'?", deleteFilename.c_str());
		ImGui::Separator();

		if (ImGui::Button("Yes"))
		{
			if (removeFile(deleteFilename) >= 0)
			{
				Toast toast
				{
					"File Deleted",
					"File '" + deleteFilename + "' was deleted successfully.",
					ImColor{ 0, 255, 0, 255 },
					Context::defaultToastDuration,
					Context::defaultToastDuration
				};

				addToastNotification(toast);
			}
			else
			{
				Toast toast
				{
					"File Not Deleted",
					"File '" + deleteFilename + "' was not deleted successfully.",
					ImColor{ 255, 0, 0, 255 },
					Context::defaultToastDuration,
					Context::defaultToastDuration
				};

				addToastNotification(toast);
			}

			deleteFilename = "";
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("No"))
		{
			deleteFilename = "";
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (overwriteFilePopup)
	{
		overwriteFilePopup = false;
		ImGui::OpenPopup("Overwrite File");
	}

	if (ImGui::BeginPopupModal("Overwrite File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure you want to overwrite '%s'?", overwriteFilename.c_str());
		ImGui::Separator();

		if (ImGui::Button("Yes"))
		{
			Toast toast
			{
				"Overwrite File",
				"File '" + overwriteFilename + "' was overwritten successfully.",
				ImColor{ 255, 0, 0, 255 },
				Context::defaultToastDuration,
				Context::defaultToastDuration
			};

			writeFile(overwriteFilename, Context::inputData);
			addToastNotification(toast);
			
			overwriteFilename = "";
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("No"))
		{
			overwriteFilename = "";
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
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
			if (validateFileName(filename))
			{
				writeFile(filename, Context::inputData);
				Toast toast
				{
					"File Saved",
					"'" + filename + "' was saved successfully.",
					ImColor{ 0, 255, 0, 255 },
					Context::defaultToastDuration,
					Context::defaultToastDuration
				};
				addToastNotification(toast);
			}
			else
			{
				Toast toast
				{
					"File Not Saved",
					"Failed to save file '" + filename + "'.",
					ImColor{ 255, 0, 0, 255 },
					Context::defaultToastDuration,
					Context::defaultToastDuration
				};

				addToastNotification(toast);
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

struct AutocompleteContext
{
	bool textWasEdited{ false };
	bool autoCloseParen{ false };
	bool skipClosingParen{ false };

	const char* buffer{ nullptr };
	std::string textToInject{};
	int replaceStart{ 0 };
	int replaceLength{ 0 };
	int cursorPos{ 0 };
};

int AutocompleteCallback(ImGuiInputTextCallbackData* data)
{
	AutocompleteContext* ctx{ static_cast<AutocompleteContext*>(data->UserData) };

	ctx->cursorPos = data->CursorPos;

	if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
	{
		if (data->EventChar == '(')
		{
			ctx->autoCloseParen = true;
			return 0;
		}

		if (data->EventChar == ')' && ctx->buffer && ctx->buffer[data->CursorPos] == ')')
		{
			ctx->skipClosingParen = true;
			return 1; 
		}
	}

	if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
	{
		ctx->textWasEdited = true;

		if (ctx->autoCloseParen)
		{
			data->InsertChars(data->CursorPos, ")");
			data->BufDirty = true;
			data->CursorPos--;
			ctx->autoCloseParen = false; 
		}
	}

	if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
	{
		if (ctx->skipClosingParen)
		{
			data->CursorPos++;
			ctx->skipClosingParen = false;
		}

		if (!ctx->textToInject.empty())
		{
			data->DeleteChars(ctx->replaceStart, ctx->replaceLength);
			data->InsertChars(ctx->replaceStart, ctx->textToInject.c_str());
			data->BufDirty = true;

			data->CursorPos = ctx->replaceStart + static_cast<int>(ctx->textToInject.length()) - 1;

			ctx->textToInject.clear();
		}
	}

	return 0;
}

struct ActiveToken
{
	std::string text{};
	int startIdx{ 0 };
	int length{ 0 };
};

ActiveToken getActiveTokenBeforeCursor(const char* buf, int cursorPos)
{
	int end{ cursorPos };
	int start{ end };

	while (start > 0 && (std::isalnum(static_cast<unsigned char>(buf[start - 1])) || buf[start - 1] == '_'))
	{
		start--;
	}

	return ActiveToken
	{
		std::string(buf + start, end - start),
		start,
		end - start
	};
}

struct CallContext
{
	std::string functionName{};
	int activeParamIndex{ 0 };
	bool insideCall{ false };
};

CallContext parseCallContext(const char* buf, int cursorPos)
{
	CallContext ctx{};
	int depth{ 0 };
	int commaCount{ 0 };
	int parenPos{ -1 };

	for (int i = cursorPos - 1; i >= 0; --i)
	{
		char c = buf[i];
		if (c == ')') depth++;
		else if (c == '(')
		{
			if (depth == 0)
			{
				parenPos = i;
				break;
			}
			depth--;
		}
		else if (c == ',' && depth == 0)
		{
			commaCount++;
		}
	}

	if (parenPos != -1)
	{
		ActiveToken funcToken = getActiveTokenBeforeCursor(buf, parenPos);
		if (!funcToken.text.empty())
		{
			ctx.functionName = funcToken.text;
			ctx.activeParamIndex = commaCount;
			ctx.insideCall = true;
		}
	}

	return ctx;
}

struct ErrorSelectionContext
{
	int start{ -1 };
	int end{ -1 };
	bool selectionDismissed{ false };
};

int ErrorSelectionCallback(ImGuiInputTextCallbackData* data)
{
	ErrorSelectionContext* ctx{ static_cast<ErrorSelectionContext*>(data->UserData) };

	if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
	{
		if (ctx->selectionDismissed)
		{
			return 0;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) ||
			ImGui::IsKeyPressed(ImGuiKey_RightArrow) ||
			ImGui::IsKeyPressed(ImGuiKey_UpArrow) ||
			ImGui::IsKeyPressed(ImGuiKey_DownArrow) ||
			ImGui::IsKeyPressed(ImGuiKey_Home) ||
			ImGui::IsKeyPressed(ImGuiKey_End))
		{
			ctx->selectionDismissed = true;
			ctx->start = -1;
			ctx->end = -1;

			data->SelectionStart = data->CursorPos;
			data->SelectionEnd = data->CursorPos;

			return 0;
		}

		if (ctx->start != -1)
		{
			data->SelectionStart = ctx->start;
			data->SelectionEnd = ctx->end;
		}
	}

	return 0;
}

// captures user input through Dear ImGui interface
void getUserInput(std::vector<Object>& object)
{
	const ImVec2 inputWindowPos{ ImGui::GetCursorScreenPos() };
	const ImVec2 inputWindowSize{ ImGui::GetContentRegionAvail() };

	static std::optional<Context::RuntimeError> diag{ std::nullopt };

	constexpr std::size_t bufferSize{ 128 };
	static char inputBuffer[bufferSize] = "";

	static ImGuiID activePopupID{ 0 };
	static int selectedIndex{ -1 };
	static bool showDropdown{ false };

	static AutocompleteContext context;
	static ErrorSelectionContext errorContext;
	static bool focusNextFrame{ false };

	ImGuiInputTextFlags inputFlags
	{
		ImGuiInputTextFlags_EnterReturnsTrue |
		ImGuiInputTextFlags_CallbackEdit |
		ImGuiInputTextFlags_CallbackAlways |
		ImGuiInputTextFlags_CallbackCharFilter
	};

	ImGuiID inputID{ ImGui::GetID("Input") };

	if (focusNextFrame)
	{
		ImGui::SetKeyboardFocusHere(0);
		focusNextFrame = false;
	}

	bool isEnterPressed{ false };

	if (diag)
	{
		if (errorContext.start != static_cast<int>(diag->charPosition) && !errorContext.selectionDismissed)
		{
			errorContext.start = static_cast<int>(diag->charPosition);
			errorContext.end = errorContext.start + static_cast<int>(diag->length);
		}
	}
	else
	{
		errorContext.start = -1;
		errorContext.end = -1;
		errorContext.selectionDismissed = false;
	}

	const char* example{ "E.g.: var = Point(1,1,1)" };
	context.buffer = inputBuffer;

	pushErrorStyle(diag);
	if (!diag)
		isEnterPressed = ImGui::InputTextWithHint("Input", example, inputBuffer, IM_COUNTOF(inputBuffer), inputFlags, AutocompleteCallback, &context);
	else
		isEnterPressed = ImGui::InputTextWithHint("Input", example, inputBuffer, IM_COUNTOF(inputBuffer), inputFlags, ErrorSelectionCallback, &errorContext);
	popErrorStyle(diag);

	if (ImGui::IsItemEdited())
	{
		diag = std::nullopt;
		errorContext.start = -1;
		errorContext.end = -1;
		errorContext.selectionDismissed = false;
	}

	const ImVec2 inputPosMin{ ImGui::GetItemRectMin() };
	const ImVec2 inputPosMax{ ImGui::GetItemRectMax() };

	CallContext callCtx = parseCallContext(inputBuffer, context.cursorPos);

	if (callCtx.insideCall && inputBuffer[0] != '\0' && !diag)
	{
		std::vector<FunctionArgs> overloads{};
		for (const auto& func : Context::function)
		{
			if (func.name == callCtx.functionName)
				overloads.push_back(func);
		}

		if (!overloads.empty())
		{
			ImGui::SetNextWindowPos(ImVec2(inputPosMin.x, inputPosMax.y + 5.0f));

			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.12f, 0.14f, 0.18f, 0.95f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 6));

			if (ImGui::BeginTooltip())
			{
				for (size_t ovIdx{ 0 }; ovIdx < overloads.size(); ++ovIdx)
				{
					const FunctionArgs& ov{ overloads[ovIdx] };

					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s(", ov.name.c_str());
					ImGui::SameLine(0, 0);

					for (size_t argIdx = 0; argIdx < ov.expectedArgs.size(); ++argIdx)
					{
						std::string argTypeName = getStringFunctionType(ov.expectedArgs[argIdx]);

						if (static_cast<int>(argIdx) == callCtx.activeParamIndex)
						{
							ImGui::TextColored(ImVec4(0.23f, 0.56f, 0.88f, 1.00f), "%s", argTypeName.c_str());
						}
						else
						{
							ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", argTypeName.c_str());
						}

						if (argIdx < ov.expectedArgs.size() - 1)
						{
							ImGui::SameLine(0, 0);
							ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), ", ");
							ImGui::SameLine(0, 0);
						}
					}

					ImGui::SameLine(0, 0);
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ")");
				}
				ImGui::EndTooltip(); 
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}
	}

	bool isInputActive{ ImGui::IsItemActive() };
	bool isInputFocused{ ImGui::IsItemFocused() };

	if (isInputActive || isInputFocused)
	{
		activePopupID = inputID;
	}

	if (context.textWasEdited && activePopupID == inputID && !diag)
	{
		showDropdown = true;
	}

	ActiveToken activeToken{ getActiveTokenBeforeCursor(inputBuffer, context.cursorPos) };
	std::vector<FunctionArgs> matches{};

	if (showDropdown && !activeToken.text.empty() && activePopupID == inputID)
	{
		for (const auto& func : Context::function)
		{
			if (func.name.rfind(activeToken.text, 0) == 0 && func.name != activeToken.text)
			{
				matches.push_back(func);
			}
		}
	}

	if (!matches.empty() && activePopupID == inputID && showDropdown)
	{
		ImGui::SetNextWindowPos(ImVec2(inputPosMin.x, inputPosMax.y));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));

		if (ImGui::BeginTooltip())
		{
			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) { selectedIndex++; }
			if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) { selectedIndex--; }

			if (selectedIndex < 0) selectedIndex = static_cast<int>(matches.size() - 1);
			if (selectedIndex >= static_cast<int>(matches.size())) selectedIndex = 0;

			for (int i{ 0 }; i < static_cast<int>(matches.size()); ++i)
			{
				const bool isSelected{ (i == selectedIndex) };
				const FunctionArgs& match{ matches[i] };

				std::string func{ match.name + "(" };

				for (std::size_t j{ 0 }; j < match.expectedArgs.size(); ++j)
				{
					const auto arg{ getStringFunctionType(match.expectedArgs[j]) };

					if (j < match.expectedArgs.size() - 1)
						func += arg + ", ";
					else
						func += arg + ")";
				}

				if (ImGui::Selectable(func.c_str(), isSelected) || (isSelected && ImGui::IsKeyPressed(ImGuiKey_Tab)))
				{
					context.textToInject = matches[i].name + "()";
					context.replaceStart = activeToken.startIdx;
					context.replaceLength = activeToken.length;
					context.textWasEdited = false;
					showDropdown = false;
					selectedIndex = -1;
					focusNextFrame = true;
				}
			}
			ImGui::EndTooltip();
		}
		ImGui::PopStyleVar();
	}

	if (showDropdown && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseClicked(0))
	{
		showDropdown = false;
		selectedIndex = -1;
	}

	if (isEnterPressed)
	{
		showDropdown = false;
		context.textWasEdited = false;
		selectedIndex = -1;

		processInput(inputBuffer, Context::function, object, diag);
		ImGui::SetKeyboardFocusHere(-1);
	}

	using Context::toastNotifications;

	if (!toastNotifications.empty())
	{
		ImGuiWindowFlags toastFlags
		{
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove
		};

		constexpr float padding{ 10.0f };

		float xPos{ inputWindowPos.x };
		float yPos{ inputWindowPos.y + inputWindowSize.y + 15.0f };

		float deltaTime{ ImGui::GetIO().DeltaTime };

		for (int i{ static_cast<int>(toastNotifications.size()) - 1 }; i >= 0; --i)
		{
			Toast& toast{ toastNotifications[static_cast<size_t>(i)] };

			toast.timeRemaining -= deltaTime;
			if (toast.timeRemaining <= 0.0f)
			{
				toastNotifications.erase(toastNotifications.begin() + i);
				continue;
			}

			float alpha{ 1.0f };
			if (toast.timeRemaining <= 0.5f)
			{
				alpha = toast.timeRemaining / 0.5f;
			}

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

			ImGui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiCond_Always);

			std::string windowID{ toast.title + "##toast_" + std::to_string(i) };

			if (ImGui::Begin(windowID.c_str(), NULL, toastFlags))
			{
				ImGui::TextColored(toast.severity, toast.message.c_str());

				yPos += ImGui::GetWindowHeight() + padding;
			}
			ImGui::End();

			ImGui::PopStyleVar();
		}
	}

	ImGui::SeparatorText("Variables");
}

void addToastNotification(const Toast& toast)
{
	Context::toastNotifications.push_back(toast);
}

void pushErrorStyle(const std::optional<Context::RuntimeError>& diag)
{
	if (!diag) return;

	if (diag->severity == Context::Error)
	{
		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(255, 0, 0, 255));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor(50, 0, 0, 255));
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(255, 255, 0, 255));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor(50, 50, 0, 255));
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
}

void popErrorStyle(const std::optional<Context::RuntimeError>& diag)
{
	if (!diag) return;

	if (ImGui::IsItemHovered()) ImGui::SetTooltip(diag->message.c_str());

	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
}

void processInput(char inputBuffer[128], const std::vector<FunctionArgs>& function, const std::vector<Object>& object, std::optional<Context::RuntimeError>& diag)
{
	std::string inputText{ inputBuffer };

	tokenizer(inputText, diag);

	if (diag)
	{
		Lexer::tokens.clear();
		return;
	}

	parser(Lexer::tokens, diag);

	if (diag)
	{
		Lexer::tokens.clear();
		Parser::nodes.clear();
		return;
	}

	int evalDelete{ evaluateDeleteFunc(diag) };
	if (evalDelete == 0)
	{
		inputBuffer[0] = '\0';
		return;
	}
	else if (evalDelete == -1)
	{
		return;
	}

	RuntimeValue evalObj{ evaluator(Parser::nodes, object) };

	if (std::holds_alternative<Context::RuntimeError>(evalObj))
	{
		Lexer::tokens.clear();
		Parser::nodes.clear();

		diag = std::get<Context::RuntimeError>(evalObj);

		return;
	}

	extractAndRegisterObject(evalObj, object, Parser::nodes, Parser::nodes[0].targetName);
	updateInputData(object[object.size() - 1]);

	if (!Context::redoBuffer.empty())
	{
		Context::redoBuffer.clear();
	}
	
	Lexer::tokens.clear();
	Parser::nodes.clear();

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

		const std::string headerText{ "##" + obj.getName() };

		bool isHeaderOpen{ ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_None) };
		
		ImGui::SameLine();
		ImGui::TextColored(ImColor{ 0, 80, 255, 255 }, obj.getName().c_str());

		ImGui::SameLine();
		ImGui::Text(std::format(": {}", getExpression(obj, object)).c_str());

		if (isHeaderOpen)
		{
			if (obj.getType() == Object::Plane || obj.getType() == Object::Line)
				ImGui::Text(getEquation(obj).c_str());

			bool valuesChanged{ getObjectInputFloats(obj) };

			static ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_None;
			ImGui::ColorEdit4(("Color###" + obj.getName()).c_str(), obj.getColorPointer(), ImGuiColorEditFlags_Float | colorFlags);

			bool colorChanged{ ImGui::IsItemDeactivatedAfterEdit() };

			if (valuesChanged || colorChanged) // saves the changes
			{
				updateObject(static_cast<int>(i), obj);
				updateInputData(obj);
			}

			std::string deleteText{ "Delete###" + std::to_string(obj.getID()) };

			if (ImGui::Button(deleteText.c_str()))
			{
				deleteObjectByID(currentID, object, Context::vertexData);
				rebuildScene(object, Context::vertexData);
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

	auto checkInput = [&](const std::string& label, float* data) 
		{
		ImGui::InputFloat3((label + obj.getName()).c_str(), data, "%.2f", textFlags);
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
			checkInput("Point###A", &point[0]);
		},
		[&](Eval::IPoint& iPoint)
		{
			checkInput("Point###A", &iPoint.point[0]);
		},
		[&](Eval::Vector& vector)
		{
			checkInput("Origin###A", &vector.origin[0]);
			checkInput("Head###B", &vector.head[0]);
		},
		[&](Eval::Segment& segment)
		{
			checkInput("A###A", &segment.A[0]);
			checkInput("B###B", &segment.B[0]);
		},
		[&](Eval::Line& line)
		{
			checkInput("Point###A", &line.point[0]);
			checkInput("DVecOrigin###B", &line.dVecOrigin[0]);
			checkInput("DVecHead###C", &line.dVecHead[0]);
		},
		[&](Eval::ILine& iLine)
		{
			checkInput("Point###A", &iLine.line.point[0]);
			checkInput("DVecOrigin###B", &iLine.line.dVecOrigin[0]);
			checkInput("DVecHead###C", &iLine.line.dVecHead[0]);
		},
		[&](Eval::Plane& plane)
		{
			checkInput("Point###A", &plane.point[0]);
			checkInput("NormalOrigin###B", &plane.normalOrigin[0]);
			checkInput("NormalHead###C", &plane.normalHead[0]);
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

void extractAndRegisterObject(const RuntimeValue& evalObj, const std::vector<Object>& object, const std::vector<Node>& nodes, const std::optional<std::string>& targetName)
{
	Object::Type type{ duduceRuntimeValueType(evalObj) };

	std::array<int, 3> pIDs{ findParentsIDs(nodes) };

	if (scanForIdenticalObject(type, evalObj, object))
	{
		std::cerr << "Object already exist.\n";
		return;
	}

	int pCount{ 0 };
	for (size_t i{ 0 }; i < std::size(pIDs); ++i)
	{
		if (pIDs[i] != -1 && pIDs[i] != Context::componentLiteral)
			++pCount;
	}

	std::string objName{};

	if (targetName) 
	{
		if (targetName->length() == 1)
		{
			int idx{ searchObjectIndexByName(*targetName, object) };

			if (idx >= 0)
			{
				Object newObj{ object[idx] };
				newObj.setComponents(evalObj);
				newObj.setParentIDs(pIDs);
				newObj.setParentCount(pCount);

				updateObject(idx, newObj);
				return;
			}

			objName = nameGen(type);
		}

		else
		{
			objName = *targetName;

			int idx{ searchObjectIndexByName(objName, object) };

			if (idx >= 0)
			{
				Object newObj{ object[idx] };
				newObj.setComponents(evalObj);
				newObj.setParentIDs(pIDs);
				newObj.setParentCount(pCount);

				updateObject(idx, newObj);
				return;
			}
		}
	}

	else 
	{
		objName = nameGen(type);
	}

	unsigned int primitive{ Context::primitives[type] };
	glm::vec4 color{ Context::defaultColors[type] };

	glm::vec4 finalColor{ color };

	// plane random color
	if (type == Object::Plane)
	{
		float r{}, g{}, b{};

		do 
		{
			r = static_cast<float>(Random::get(0, 10)) * 0.1f;
			g = static_cast<float>(Random::get(0, 10)) * 0.1f;
			b = static_cast<float>(Random::get(0, 10)) * 0.1f;
		} 
		while (r < 0.2f && g > 0.4f && b > 0.7f);

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
}

void debugWindow()
{
	using namespace Context;

	if (ImGui::Begin("DebugWindow", NULL))
	{
		if (ImGui::CollapsingHeader("Context", NULL))
		{
			ImGui::Indent();

			ImGui::SeparatorText("Flags");
			ImGui::Text(std::format("isPressingRightClick: {}", isPressingRightClick).c_str());
			ImGui::Text(std::format("isFirstMouse: {}", isFirstMouse).c_str());
			ImGui::Text(std::format("isEnterPressed: {}", isEnterPressed).c_str());
			ImGui::Text(std::format("leftClickPressed: {}", leftClickPressed).c_str());

			ImGui::SeparatorText("Numbers");
			ImGui::Text(std::format("fov: {}", fov).c_str());
			ImGui::Text(std::format("prevSelectedObjID: {}", prevSelectedObjID).c_str());
			ImGui::Text(std::format("selectedObjID: {}", selectedObjID).c_str());
			ImGui::Text(std::format("globalObjectIDCounter: {}", globalObjectIDCounter).c_str());

			ImGui::SeparatorText("Vertex Data");
			ImGui::Text(std::format("Size: {}", vertexData.size()).c_str());
			ImGui::Text(std::format("Capacity: {}", vertexData.capacity()).c_str());

			ImGui::SeparatorText("Text");
			ImGui::Text(std::format("inputData:\n{}", inputData).c_str());
			
			std::string finalText{};
			for (const auto& str : redoBuffer)
			{
				finalText += str;
			}
			ImGui::Text(std::format("redoBuffer:\n{}", finalText).c_str());

			ImGui::SeparatorText("Symbol Table");

			for (const auto& [key, value] : symbolTable)
			{
				ImGui::Text(std::format("{}: {}", key, value).c_str());
			}

			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Objects", NULL))
		{
			ImGui::Indent();

			ImGui::Text(std::format("Object size: {}", object.size()).c_str());
			ImGui::SeparatorText("Variables");

			for (size_t idx{ 0 }; idx < object.size(); ++idx)
			{
				const Object& obj{ object[idx] };

				if (ImGui::CollapsingHeader((obj.getName() + "###" + std::to_string(obj.getID())).c_str(), NULL))
				{
					ImGui::SeparatorText("Numbers");
					ImGui::Text(std::format("Index: {}", idx).c_str());
					ImGui::Text(std::format("id: {}", obj.getID()).c_str());
					ImGui::Text(std::format("offset: {}", obj.getOffset()).c_str());
					ImGui::Text(std::format("vertexCount: {}", obj.getVertexCount()).c_str());
					ImGui::Text(std::format("parentCount: {}", obj.getParentCount()).c_str());

					ImGui::SeparatorText("Flags");
					ImGui::Text(std::format("isMutable: {}", obj.isMutable()).c_str());
					ImGui::Text(std::format("isSelected: {}", obj.isSelected()).c_str());

					ImGui::SeparatorText("Other");
					const std::array<int, 3>& pIDs{ obj.getParentIDs() };
					ImGui::Text(std::format("parentIDs: [{}, {}, {}]", pIDs[0], pIDs[1], pIDs[2]).c_str());
				}
			}
			ImGui::Unindent();
		}
	}

	ImGui::End();
}