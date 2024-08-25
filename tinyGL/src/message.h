#pragma once
#include <map>

struct GLFWwindow;
namespace Kong
{

class CMessage
{
public:
	static void KeyCallback(GLFWwindow* window, int key, int scan_code, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mode);

	static void BindKeyToFunction(int key, int action, void(*func)());
	static void BindMouseToFunction(int key, int action, void(*func)());

private:
	GLFWwindow* m_pWindow;

	std::map<std::pair<int, int>, void(*)()> m_mKeyFuncMap;
	std::map<std::pair<int, int>, void(*)()> m_mMouseFuncMap;
};

CMessage* GetMessageHandler();
}