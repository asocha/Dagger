//=====================================================
// MessageBar.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_MessageBar__
#define __included_MessageBar__

#include "Engine/Console/Console.hpp"

class MessageBar{
private:
	Console m_textRenderer;

public:
	bool m_isWaitingForPlayer;
	std::string m_text;
	std::vector<std::string> m_messages;

	MessageBar();

	void Startup(OpenGLRenderer* renderer);
	void Shutdown(const OpenGLRenderer* renderer);

	void Render(const OpenGLRenderer* renderer);
};
extern MessageBar* s_theMessageBar;

#endif