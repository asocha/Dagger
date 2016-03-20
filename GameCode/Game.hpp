//=====================================================
// Game.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_World__
#define __included_World__

class OpenGLRenderer;
class BaseMapGenerator;
#include "Engine/Console/Console.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Entities/Actor.hpp"
class Map;
class Player;
#include "StatusBar.hpp"
#include "MessageBar.hpp"
#include "AISystem.hpp"

enum GameState{
	MainMenu,
	Quest,
	Generating,
	Playing,
	Defeat,
	Victory
};

class Game{
private:
	bool m_savedGame;
	bool m_isRunning;
	GameState m_gameState;
	AISystem m_AISystem;

	Console m_titleRenderer;
	Console m_menuRenderer;
	Console m_bottomMessageRenderer;

	StatusBar m_statusBar;
	MessageBar m_messageBar;

	EngineAndrew::Material m_HUDMaterial;
	int m_HUDIndexBufferID;
	int m_HUDVaoID;
	int m_HUDBufferID;

	Map* m_map;
	BaseMapGenerator* m_mapGenerator;

	Player* m_player;

	bool m_autoGenerateMap;
	bool m_forceMapVisible;

	void SaveGame();
	void LoadGame(OpenGLRenderer* renderer);
	void ResolvePointersFromLoadGame() const;

	void SetState(GameState state);

	void UpdateMainMenuState(OpenGLRenderer* renderer);
	void UpdateQuestState(OpenGLRenderer* renderer);
	void UpdateGeneratingState(OpenGLRenderer* renderer);
	void UpdatePlayingState(OpenGLRenderer* renderer);
	bool UpdatePlayer(OpenGLRenderer* renderer);
	void UpdateGameOverState();

	void HandleDeadEntities(const OpenGLRenderer* renderer) const;

	void DestroyCurrentGame(const OpenGLRenderer* renderer);

public:
	Game();
	
	void Draw(OpenGLRenderer* renderer);
	void Update(OpenGLRenderer* renderer);

	void Startup(OpenGLRenderer* renderer);
	void Shutdown(const OpenGLRenderer* renderer);
	inline bool IsRunning() const{return m_isRunning;}
};

#endif