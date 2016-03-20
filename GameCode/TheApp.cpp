//=====================================================
// TheApp.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "TheApp.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Renderer/OpenGLRenderer.hpp"
#include "Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Sound/SoundSystem.hpp"
#include "Engine/Console/Console.hpp"
#include "Engine/Console/ConsoleCommands.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Core/ProfileSection.hpp"
#include <algorithm>
#include "Engine/Threads/JobManager.hpp"
#include "Engine/Core/SignpostMemoryManager.hpp"

///=====================================================
/// 
///=====================================================
TheApp::TheApp()
	:m_isRunning(true),
	m_world(nullptr),
	m_renderer(nullptr),
	m_inputSystem(nullptr),
	m_soundSystem(nullptr),
	m_console(nullptr),
	m_masterClock(nullptr){
}

///=====================================================
/// 
///=====================================================
void TheApp::Startup(void* windowHandle){
	m_windowHandle = windowHandle;

	InitializeTimer();

	m_masterClock = new Clock(nullptr);
	RECOVERABLE_ASSERT(m_masterClock != nullptr);

	m_inputSystem = new InputSystem();
	RECOVERABLE_ASSERT(m_inputSystem != nullptr);
	if (m_inputSystem){
		m_inputSystem->Startup(windowHandle);
		m_inputSystem->ShowMouse(false);
	}

	m_soundSystem = new SoundSystem();
	RECOVERABLE_ASSERT(m_soundSystem != nullptr);
	if (m_soundSystem){
		bool success = m_soundSystem->Startup();
		if (!success){
			m_soundSystem->Shutdown();
			delete m_soundSystem;
			m_soundSystem = nullptr;
		}
	}

	m_renderer = new OpenGLRenderer();
	RECOVERABLE_ASSERT(m_renderer != nullptr);
	if (m_renderer){
		m_renderer->Startup((HWND)windowHandle);
		m_renderer->CheckGLCompatibility();
		m_renderer->SetAlphaTest(true);
		m_renderer->IgnoreEmptyPixels();

		m_console = new Console();
		RECOVERABLE_ASSERT(m_console != nullptr);
		if (m_console != nullptr){
			m_console->Startup(m_renderer);
			m_console->m_isVisible = false;
		}

		ProfileSection::StartupProfiling(m_renderer);

		m_world = new Game();
		RECOVERABLE_ASSERT(m_world != nullptr);
		if (m_world != nullptr){
			m_world->Startup(m_renderer);
		}
		else{
			m_isRunning = false;
		}
	}
	else{
		m_isRunning = false;
	}
}

///=====================================================
/// 
///=====================================================
void TheApp::Run(){
	while (m_isRunning){
		ProcessInput();
		Update();
		RenderWorld();
		if (!m_renderer->IsRunning())
			m_isRunning = false;
	}
}

///=====================================================
/// 
///=====================================================
void TheApp::Shutdown(){
	if (m_masterClock)
		delete m_masterClock;

	if (m_world){
		m_world->Shutdown(m_renderer);
		delete m_world;
	}

	if (m_console){
		m_console->Shutdown(m_renderer);
		delete m_console;
	}

	ProfileSection::ShutdownProfiling(m_renderer);

	if (m_renderer){
		m_renderer->Shutdown();
		delete m_renderer;
	}

	if (m_inputSystem){
		m_inputSystem->Shutdown();
		delete m_inputSystem;
	}

	if (m_soundSystem){
		m_soundSystem->Shutdown();
		delete m_soundSystem;
	}

	if (s_theCommandList)
		delete s_theCommandList;
}

CONSOLE_COMMAND(QUIT){
	if (args->m_args != nullptr) return false;

	s_theInputSystem->SetSystemQuit();
	return true;
}

CONSOLE_COMMAND(PROFILE){
	if (args->m_args != nullptr){
		if (args->m_args[0] != "1")
			return false;

		std::string argument = args->m_args[1];
		std::transform(argument.begin(), argument.end(), argument.begin(), ::tolower);

		if (argument == "average"){
			ProfileSection::SetRendering(true);
			ProfileSection::SetUseAverages(true);
		}
		else if (argument == "current"){
			ProfileSection::SetRendering(true);
			ProfileSection::SetUseAverages(false);
		}
		else{
			return false;
		}
	}
	else{
		ProfileSection::ToggleRendering();
	}

	return true;
}

///=====================================================
/// 
///=====================================================
void TheApp::ProcessInput(){
	PROFILE_CATEGORY(Input);
	if (m_inputSystem){
		m_inputSystem->Update();

		if (/*(m_inputSystem->IsKeyDown(VK_ESCAPE) && !m_console->m_isVisible) || */m_inputSystem->IsReadyToQuit()){
			m_isRunning = false;
		}
		else if (m_inputSystem->GetKeyWentDown(192)){ // ` key
			m_console->m_isVisible = !m_console->m_isVisible;

			if (m_console->m_isVisible){
				m_console->CreateInputString();
			}
			else{
				m_console->DeleteInputString();
			}
		}
		else if (m_console->m_isVisible){
			if (m_inputSystem->GetKeyWentDown(VK_UP)){
				m_console->ScrollUpTextHistory();
			}
			else if (m_inputSystem->GetKeyWentDown(VK_DOWN)){
				m_console->ScrollDownTextHistory();
			}
			else{
				int currentCharacterDown = m_inputSystem->GetCurrentCharacterDown();
				if (currentCharacterDown == VK_BACK){ //Backspace
					m_console->DeleteFromInputString();
				}
				else if (currentCharacterDown == VK_ESCAPE){ //Escape
					bool didDelete = m_console->ClearInputString();
					m_console->m_isVisible = didDelete;
				}
				else if (currentCharacterDown == VK_RETURN){ //Enter
					if (m_console->HasInputString()){
						m_console->ExecuteInputString();
					}
					else{
						m_console->m_isVisible = false;
					}
				}
				else if (currentCharacterDown != -1){ //any other key
					m_console->AppendToInputString((unsigned char)currentCharacterDown);
				}
			}
			m_inputSystem->ResetKeys(); //prevent normal input while console is open
		}
	}
}

///=====================================================
/// 
///=====================================================
void TheApp::Update(){
	double currentTime = GetCurrentSeconds();
	static double lastTime = currentTime;
	float deltaSeconds = (float)(currentTime - lastTime);

	lastTime = currentTime;

	m_masterClock->AdvanceTime(deltaSeconds);

	if (s_theJobManager){
		s_theJobManager->Update();
	}

	if (m_soundSystem){
		m_soundSystem->Update();
	}

	if (m_world){
		m_world->Update(m_renderer);

		if (!m_world->IsRunning())
			m_isRunning = false;
	}
}

///=====================================================
/// 
///=====================================================
void TheApp::RenderWorld() const{
	m_renderer->ClearBuffer();

	m_renderer->SetDepthTest(true);

	if (m_world != nullptr)
		m_world->Draw(m_renderer);

	m_console->RenderText(m_renderer, Font::ARIAL, 32.0f, Vec2(10.0f, 10.0f));

	if (s_theMemoryManager != nullptr)
		s_theMemoryManager->FinishFrame();

	ProfileSection::Render(m_renderer);
	ProfileSection::FinishProfilingFrame();

	m_renderer->SwapBuffers();
}