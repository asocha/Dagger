//=====================================================
// StatusBar.cpp
// by Andrew Socha
//=====================================================

#include "StatusBar.hpp"
#include "Entities/Player.hpp"


///=====================================================
/// 
///=====================================================
StatusBar::StatusBar():
m_textRenderer(),
m_player(nullptr),
m_turnCount(0){
}

///=====================================================
/// 
///=====================================================
void StatusBar::Startup(OpenGLRenderer* renderer) {
	m_textRenderer.Startup(renderer);

	m_textRenderer.ToggleCursor();
	m_textRenderer.ToggleRenderFromCenter();

	m_textRenderer.CreateInputString();
}

///=====================================================
/// 
///=====================================================
void StatusBar::Shutdown(const OpenGLRenderer* renderer) {
	m_textRenderer.Shutdown(renderer);
}

///=====================================================
/// 
///=====================================================
void StatusBar::Render(const OpenGLRenderer* renderer){
	if (m_player == nullptr){
		RECOVERABLE_ERROR();
		return;
	}

	m_textRenderer.DeleteInputString();

	m_textRenderer.Printf("Andrew                 Health: %i/%i                 Turns: %i", m_player->m_health, m_player->m_maxHealth, m_player->s_turnCount);

	m_textRenderer.RenderText(renderer, Font::CONSOLAS, 32.0f, Vec2(800.0f, 50.0f));
}
