//=====================================================
// MessageBar.cpp
// by Andrew Socha
//=====================================================

#include "MessageBar.hpp"

MessageBar* s_theMessageBar = nullptr;

///=====================================================
/// 
///=====================================================
MessageBar::MessageBar() :
m_textRenderer(),
m_text(""),
m_messages(),
m_isWaitingForPlayer(false){
	RECOVERABLE_ASSERT(s_theMessageBar == nullptr);
	s_theMessageBar = this;
}

///=====================================================
/// 
///=====================================================
void MessageBar::Startup(OpenGLRenderer* renderer) {
	m_textRenderer.Startup(renderer);

	m_textRenderer.ToggleCursor();
	m_textRenderer.ToggleRenderFromCenter();
	m_textRenderer.CreateInputString();
}

///=====================================================
/// 
///=====================================================
void MessageBar::Shutdown(const OpenGLRenderer* renderer) {
	m_textRenderer.Shutdown(renderer);
}

///=====================================================
/// 
///=====================================================
void MessageBar::Render(const OpenGLRenderer* renderer){
	std::string tempText = m_text;

	if (m_isWaitingForPlayer)
		goto renderText;

	m_textRenderer.DeleteInputString();

	for (std::vector<std::string>::iterator messageIter = m_messages.begin(); messageIter != m_messages.end();){
		tempText += *messageIter;
		if (Console::CalcTextWidth(tempText, Font::CONSOLAS, 32.0f) > 1300.0f){
			m_text += "--More--";
			m_isWaitingForPlayer = true;
			break;
		}
		tempText += " ";
		m_text = tempText;

		messageIter = m_messages.erase(messageIter);
	}

	m_textRenderer.PrintText(m_text);

renderText:
	m_textRenderer.RenderText(renderer, Font::CONSOLAS, 32.0f, Vec2(800.0f, 850.0f));
}
