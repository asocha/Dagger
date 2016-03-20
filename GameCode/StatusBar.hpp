//=====================================================
// StatusBar.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_StatusBar__
#define __included_StatusBar__

#include "Engine/Console/Console.hpp"
class Player;

class StatusBar{
private:
	Console m_textRenderer;
	int m_turnCount;

public:
	Player* m_player;

	StatusBar();

	void Startup(OpenGLRenderer* renderer);
	void Shutdown(const OpenGLRenderer* renderer);
	
	void Render(const OpenGLRenderer* renderer);
};

#endif