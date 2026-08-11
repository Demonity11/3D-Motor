#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class Window
{
public:
	Window(int width, int height, const std::string& title);
	~Window();
	bool init();
	void clear(float red, float green, float blue, float alpha);

	GLFWwindow* getWindow() const { return m_window; }

private:
	int m_width{};
	int m_height{};
	std::string m_title{};
	GLFWwindow* m_window{};

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
};

#endif