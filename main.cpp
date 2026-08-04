#include "Window.h"
#include "Shader.h"
#include "draw_utils.h"
#include "objectCoords.h"
#include "utilities.h"
#include "Context.h"
#include "evaluator.h"

#include "fileManager.h"

void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

struct TransparentItem
{
	int objIndex{};
	float alpha{};
};

struct RenderItem
{
	size_t objIndex{};
	float distance{};
	float alpha{};
};

int main()
{
	constexpr int width{ 960 };
	constexpr int height{ 540 };
	std::string title{ "3[D] Motor" };

	Window window{ width, height, title };
	window.init();

	Shader shader{ "shader.vert", "shader.frag" };

	glfwSetMouseButtonCallback(window.getWindow(), mouse_button_callback);
	glfwSetCursorPosCallback(window.getWindow(), mouse_cursor_callback);
	glfwSetScrollCallback(window.getWindow(), mouse_scroll_callback);

	getEnvironmentVertices(Context::vertexData, true);
	vertexSpec(Context::vertexData);

	initializeImGui(window.getWindow());
	setupCustomTheme();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		processInput(window.getWindow());

		// ImGui stuff
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiIO& io{ ImGui::GetIO() };
		window.clear(0.0f, 0.5f, 0.8f, 1.0f);
		shader.use();

		glm::mat4 view{ glm::lookAt(Context::cameraPos, Context::cameraTarget, Context::worldUp) };

		glm::mat4 projection{};
		projection = glm::perspective(glm::radians(Context::fov), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

		glm::mat4 model{ 1.0f };

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat4("model", model);

		glBindVertexArray(Context::VAO);

		std::vector<RenderItem> transparentQueue{};
		transparentQueue.reserve(Context::object.size());

		std::vector<RenderItem> opaqueQueue{};
		opaqueQueue.reserve(Context::object.size());

		for (size_t i{ 0 }; i < Context::object.size(); ++i)
		{
			const Object& obj{ Context::object[i] };
			float distance{ getDistanceToCamera(Context::cameraPos, obj) };

			if (distance < 0.0f) continue;

			const float alpha{ obj.getColor().w };

			if (glm::abs(1.0f - alpha) < FLT_EPSILON)
			{
				opaqueQueue.emplace_back(RenderItem{ i, distance, alpha });
			}
			else
			{
				transparentQueue.emplace_back(RenderItem{ i, distance, alpha });
			}
		}

		std::sort(opaqueQueue.begin(), opaqueQueue.end(),
			[](const RenderItem& a, const RenderItem& b)
			{
				return a.distance < b.distance;
			}
		);

		std::sort(transparentQueue.begin(), transparentQueue.end(),
			[](const RenderItem& a, const RenderItem& b)
			{
				return a.distance > b.distance;
			}
		);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		for (const auto& ri : opaqueQueue)
		{
			const Object& obj{ Context::object[ri.objIndex] };

			glDrawArrays(obj.getPrimitive(), obj.getOffset(), obj.getVertexCount());
		}

		glDepthMask(GL_FALSE);

		for (const auto& ri : transparentQueue)
		{
			const Object& obj{ Context::object[ri.objIndex] };

			glDrawArrays(obj.getPrimitive(), obj.getOffset(), obj.getVertexCount());
		}

		glDepthMask(GL_TRUE);

		// render ImGui here
		ImGui::ShowDemoWindow();
		undoRedoShortcut();

		ImGui::PushFont(Context::spaceFont, 16.0f);
		ImGui::Begin("InputWindow", NULL, ImGuiWindowFlags_MenuBar);
		getUserInput(Context::object);
		menuBar();

		int windowWidth{};
		int windowHeight{};
		glfwGetWindowSize(window.getWindow(), &windowWidth, &windowHeight);

		glm::vec2 viewportPos{ 0.0f, 0.0f }; 
		glm::vec2 viewportSize{ static_cast<float>(windowWidth), static_cast<float>(windowHeight) };

		drawObjectLabels(Context::object, view, projection, model, viewportPos, viewportSize);
		drawAxisLabels(Context::object, view, projection, model, viewportPos, viewportSize);

		if (Context::leftClickPressed && !io.WantCaptureMouse)
		{
			Context::leftClickPressed = false;

			ImVec2 imguiMousePos{ ImGui::GetMousePos() };

			float mouseX{ imguiMousePos.x };
			float mouseY{ imguiMousePos.y };

			float ndcX{ (2.0f * (mouseX - viewportPos.x)) / viewportSize.x - 1.0f };
			float ndcY{ 1.0f - (2.0f * (mouseY - viewportPos.y)) / viewportSize.y };

			glm::vec4 rayNearNDC{ ndcX, ndcY, -1.0f, 1.0f };
			glm::vec4 rayFarNDC{ ndcX, ndcY, 1.0f, 1.0f };

			glm::mat4 invMVP{ glm::inverse(projection * view * model) };

			glm::vec4 rayNearLocal{ invMVP * rayNearNDC };
			glm::vec4 rayFarLocal{ invMVP * rayFarNDC };

			glm::vec3 pLocalNear{ rayNearLocal / rayNearLocal.w };
			glm::vec3 pLocalFar{ rayFarLocal / rayFarLocal.w };

			glm::vec3 rayOrigin{ pLocalNear };
			glm::vec3 rayDirection{ glm::normalize(pLocalFar - pLocalNear) };

			int clickedID{ getSelectedObjectID(rayOrigin, rayDirection, Context::object) };
			
			if (Context::selectedObjID == clickedID)
			{
				Context::selectedObjID = -1; 
			}
			else
			{
				Context::selectedObjID = clickedID;
			}
		}

		if (io.WantCaptureMouse)
		{
			Context::leftClickPressed = false;
		}

		showVariables(Context::object);
		ImGui::PopFont();
		ImGui::End();

		if (Context::debugWindow)
		{
			debugWindow();
		}

		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window.getWindow());
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void processInput(GLFWwindow* window)
{
	//if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
	//	glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT == GLFW_PRESS))
	{
		Context::isPressingRightClick = true;
		Context::isFirstMouse = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT == GLFW_RELEASE))
		Context::isPressingRightClick = false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			Context::isPressingRightClick = true;
			Context::isFirstMouse = true;
		}

		if (action == GLFW_RELEASE)
			Context::isPressingRightClick = false;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			Context::leftClickPressed = true;
		}
	}
}

void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!Context::isPressingRightClick)
		return;


	if (Context::isFirstMouse)
	{
		Context::lastX = static_cast<float>(xpos);
		Context::lastY = static_cast<float>(ypos);
		Context::isFirstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - Context::lastX;
	float yoffset = static_cast<float>(ypos) - Context::lastY;

	Context::lastX = static_cast<float>(xpos);
	Context::lastY = static_cast<float>(ypos);

	const float sensitivity{ 0.01f };

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	Context::yaw += xoffset;
	Context::pitch += yoffset;

	if (Context::pitch > 89.0f)
		Context::pitch = 89.0f;

	if (Context::pitch < -89.0f)
		Context::pitch = -89.0f;

	const float radius{ 3.0f };

	Context::cameraPos.x = Context::cameraTarget.x + radius * cos(Context::yaw) * cos(Context::pitch);
	Context::cameraPos.y = Context::cameraTarget.y + radius * sin(Context::pitch);
	Context::cameraPos.z = Context::cameraTarget.z + radius * sin(Context::yaw) * cos(Context::pitch);
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Context::fov -= static_cast<float>(yoffset) * 1.3f;

	if (Context::fov < 25.0f)
		Context::fov = 25.0f;
	if (Context::fov > 45.0f)
		Context::fov = 45.0f;
}

void vertexSpec(const std::vector<float>& vertices)
{
	glGenVertexArrays(1, &Context::VAO);
	glBindVertexArray(Context::VAO);

	glGenBuffers(1, &Context::VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Context::VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

	int stride{ 7 * sizeof(float) };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
}

void updateBufferData(const std::vector<float>& vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, Context::VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
}
