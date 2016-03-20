//=====================================================
// Game.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/OpenGLRenderer.hpp"
#include "Generators/BaseMapGenerator.hpp"
#include "Map.hpp"
#include "Entities/Player.hpp"
#include "Factories/NPCFactory.hpp"
#include "Factories/ItemFactory.hpp"
#include "Factories/FeatureFactory.hpp"
#include "AIBehaviors/BaseAIBehavior.hpp"
#include "Faction.hpp"
#include "Entities/Feature.hpp"
#include "Engine/Core/Utilities.hpp"
#include <time.h>
#include "Entities/NPC.hpp"
#include <assert.h>
#include "Engine/Core/ProfileSection.hpp"


///=====================================================
/// 
///=====================================================
Game::Game()
:m_isRunning(true),
m_gameState(MainMenu),
m_HUDMaterial(),
m_HUDIndexBufferID(0),
m_HUDVaoID(0),
m_HUDBufferID(0),
m_menuRenderer(),
m_titleRenderer(),
m_bottomMessageRenderer(),
m_map(nullptr),
m_mapGenerator(nullptr),
m_player(nullptr),
m_forceMapVisible(false),
m_autoGenerateMap(true),
m_statusBar(),
m_messageBar(),
m_savedGame(false),
m_AISystem(){
}

///=====================================================
/// 
///=====================================================
void Game::Startup(OpenGLRenderer* renderer){
	srand((unsigned int)time(NULL));
	Faction::LoadFactions();

	bool loadedNPCs = NPCFactory::LoadAllNPCFactories(renderer);
	if (!loadedNPCs)
		return;

	bool loadedItems = ItemFactory::LoadAllItemFactories(renderer);
	if (!loadedItems)
		return;

	bool loadedFeatures = FeatureFactory::LoadAllFeatureFactories(renderer);
	if (!loadedFeatures)
		return;

	std::vector<std::string> unused;
	bool hasSavedGame = FindAllFilesOfType("Data/Saves/", "saveGame.xml", unused);

	m_titleRenderer.Startup(renderer);
	m_titleRenderer.ToggleCursor();
	m_titleRenderer.ToggleRenderFromCenter();

	m_titleRenderer.PrintText("Dagger", RGBAchars::RED);


	m_menuRenderer.Startup(renderer);
	m_menuRenderer.ToggleCursor();
	m_menuRenderer.ToggleRenderFromCenter();
	
	m_menuRenderer.PrintText("N) New Game");
	if (hasSavedGame)
		m_menuRenderer.PrintText("L) Load Game");
	m_menuRenderer.PrintText("Q) Quit");

	m_bottomMessageRenderer.Startup(renderer);
	m_bottomMessageRenderer.ToggleCursor();
	m_bottomMessageRenderer.ToggleRenderFromCenter();

	m_statusBar.Startup(renderer);
	m_messageBar.Startup(renderer);

	//Create HUD decoration
	bool wasCreated = m_HUDMaterial.CreateProgram(renderer, "Data/Shaders/basic2DNoTexture.vert", "Data/Shaders/basic2DNoTexture.frag");
	if (wasCreated == false){
		m_isRunning = false;
		return;
	}

	Vertex2D_PCT boxVerts[4] = {
		Vertex2D_PCT(Vec2(100.0f, 100.0f), Vec2(0.0f, 1.0f)),
		Vertex2D_PCT(Vec2(1500.0f, 100.0f), Vec2(1.0f, 1.0f)),
		Vertex2D_PCT(Vec2(100.0f, 800.0f), Vec2(0.0f, 0.0f)),
		Vertex2D_PCT(Vec2(1500.0f, 800.0f), Vec2(1.0f, 0.0f))
	};

	int boxIndeces[8] = {
		0, 1, 1, 3, 3, 2, 2, 0
	};

	renderer->GenerateBuffer((GLuint*)&m_HUDBufferID);
	renderer->SendVertexDataToBuffer(&boxVerts, sizeof(Vertex2D_PCT) * 4, m_HUDBufferID);

	renderer->GenerateBuffer((GLuint*)&m_HUDIndexBufferID);
	renderer->SendVertexDataToBuffer(&boxIndeces, sizeof(int) * 8, m_HUDIndexBufferID);

	m_HUDVaoID = renderer->CreateVAOBasic();

	m_HUDMaterial.BindVertexData(m_HUDBufferID, Vertex2D_PCT());
	m_HUDMaterial.CreateSampler(renderer, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	m_HUDMaterial.SetBaseShape(GL_LINES);

	UniformMatrix* projection = (UniformMatrix*)m_HUDMaterial.CreateUniform("projection");
	FATAL_ASSERT(projection != nullptr);
	projection->m_data.push_back(renderer->CreateOrthographicMatrix());
}

///=====================================================
/// 
///=====================================================
void Game::Shutdown(const OpenGLRenderer* renderer){
	m_isRunning = false;

	m_titleRenderer.Shutdown(renderer);
	m_menuRenderer.Shutdown(renderer);

	m_statusBar.Shutdown(renderer);
	m_messageBar.Shutdown(renderer);

	if (m_map != nullptr){
		m_map->DestroyMap(renderer);
		delete m_map;
		delete m_mapGenerator;
	}

	delete MapGeneratorRegistration::GetGeneratorRegistry();
	delete AIBehaviorRegistration::GetBehaviorRegistry();

	NPCFactory::DeleteNPCFactories();
	ItemFactory::DeleteItemFactories();
	FeatureFactory::DeleteFeatureFactories();
	Faction::DeleteFactions();

	renderer->DeleteBuffer((GLuint*)&m_HUDBufferID);
	renderer->DeleteBuffer((GLuint*)&m_HUDIndexBufferID);
	renderer->DeleteVAO((GLuint*)&m_HUDVaoID);
}

///=====================================================
/// 
///=====================================================
void Game::SaveGame(){
	XMLNode saveNode = XMLNode::createXMLTopNode("SaveGame");

	XMLNode mapNode = saveNode.addChild("Map");
	XMLNode mapVisibilityNode = saveNode.addChild("Visibility");
	m_map->SaveGame(mapNode, mapVisibilityNode);

	XMLNode entitiesNode = saveNode.addChild("Entities");
	for (Actors::iterator actorIter = Actor::s_actorsOnMap.begin(); actorIter != Actor::s_actorsOnMap.end(); ++actorIter){
		Actor* actor = actorIter->second;
		actor->SaveGame(entitiesNode);
	}
	for (Items::iterator itemIter = Item::s_itemsOnMap.begin(); itemIter != Item::s_itemsOnMap.end(); ++itemIter){
		Item* item = *itemIter;
		item->SaveGame(entitiesNode);
	}
	for (Features::iterator featureIter = Feature::s_featuresOnMap.begin(); featureIter != Feature::s_featuresOnMap.end(); ++featureIter){
		Feature* feature = *featureIter;
		feature->SaveGame(entitiesNode);
	}


	std::string saveFile = "Data/Saves/saveGame.xml";
	saveNode.writeToFile(saveFile.c_str());

	m_messageBar.m_text.clear();
	m_messageBar.m_messages.clear();
	m_messageBar.m_messages.push_back("Game Saved to " + saveFile + "   Goodbye!!!!");
	m_messageBar.m_messages.push_back("                                              "); //force a --MORE--

	m_savedGame = true;
}

///=====================================================
/// 
///=====================================================
void Game::LoadGame(OpenGLRenderer* renderer){
	XMLNode root = XMLNode::parseFile("Data/Saves/saveGame.xml");
	if (root.isEmpty())
		return;

	remove("Data/Saves/saveGame.xml");

	XMLNode savedGame = root.getChildNode("SaveGame");

	SetState(Playing);

	const MapGeneratorRegistry* registry = MapGeneratorRegistration::GetGeneratorRegistry();

	m_mapGenerator = registry->at("FromData")->CreateMapGenerator("FromData");
	if (m_mapGenerator == nullptr){
		assert(false);
		return;
	}

	m_map = m_mapGenerator->CreateMap(renderer, savedGame);
	if (m_map == nullptr){
		assert(false);
		return;
	}

	XMLNode entitiesNode = savedGame.getChildNode("Entities");
	m_mapGenerator->FinalizeMap(m_map, entitiesNode);

	XMLNode playerNode = entitiesNode.getChildNode("Player");
	m_player = new Player(Player(renderer), renderer, playerNode);
	m_player->AddToMap(m_map, m_player->m_position);
	m_statusBar.m_player = m_player;

	ResolvePointersFromLoadGame();
}

///=====================================================
/// 
///=====================================================
void Game::ResolvePointersFromLoadGame() const{
	for (Actors::iterator actorIter = Actor::s_actorsOnMap.begin(); actorIter != Actor::s_actorsOnMap.end(); ++actorIter){
		Actor* actor = actorIter->second;
		actor->ResolvePointersFromLoadGame();
	}
	for (Items::iterator itemIter = Item::s_itemsOnMap.begin(); itemIter != Item::s_itemsOnMap.end(); ++itemIter){
		Item* item = *itemIter;
		item->ResolvePointersFromLoadGame();
	}
	for (Features::iterator featureIter = Feature::s_featuresOnMap.begin(); featureIter != Feature::s_featuresOnMap.end(); ++featureIter){
		Feature* feature = *featureIter;
		feature->ResolvePointersFromLoadGame();
	}
}

///=====================================================
/// 
///=====================================================
void Game::SetState(GameState state){
	if (m_gameState != state){
		m_gameState = state;
		switch (m_gameState){
		case MainMenu:
			s_theConsole->PrintText("Change State: Main Menu");
			break;
		case Quest:
			s_theConsole->PrintText("Change State: Quest Screen");
			break;
		case Generating:
			s_theConsole->PrintText("Change State: Generating World");
			break;
		case Playing:
			s_theConsole->PrintText("Change State: Playing");
			break;
		case Defeat:
			s_theConsole->PrintText("Change State: Defeat");
			break;
		case Victory:
			s_theConsole->PrintText("Change State: Victory");
			break;
		}
	}
	else{
		switch (m_gameState){
		case MainMenu:
			s_theConsole->PrintText("Warning: Switch to Already Present State, Main Menu", RGBAchars::RED);
			break;
		case Quest:
			s_theConsole->PrintText("Warning: Switch to Already Present State, Quest Screen", RGBAchars::RED);
			break;
		case Generating:
			s_theConsole->PrintText("Warning: Switch to Already Present State, Generating World", RGBAchars::RED);
			break;
		case Playing:
			s_theConsole->PrintText("Warning: Switch to Already Present State, Playing", RGBAchars::RED);
			break;
		case Defeat:
			s_theConsole->PrintText("Warning: Switch to Already Present State, Defeat", RGBAchars::RED);
			break;
		case Victory:
			s_theConsole->PrintText("Warning: Switch to Already Present State, Victory", RGBAchars::RED);
			break;
		}
	}
}


///=====================================================
/// 
///=====================================================
void Game::Draw(OpenGLRenderer* renderer){
	switch (m_gameState){
	case MainMenu:
		m_titleRenderer.RenderText(renderer, Font::CONSOLAS, 150.0f, Vec2(800.0f, 450.0f));
		m_menuRenderer.RenderText(renderer, Font::CONSOLAS, 50.0f, Vec2(800.0f, 250.0f));
		break;
	case Quest:
		m_HUDMaterial.Render(m_HUDVaoID, m_HUDIndexBufferID, 8);
		m_titleRenderer.RenderText(renderer, Font::CONSOLAS, 150.0f, Vec2(800.0f, 700.0f));
		m_menuRenderer.RenderText(renderer, Font::CONSOLAS, 50.0f, Vec2(150.0f, 500.0f));
		m_bottomMessageRenderer.RenderText(renderer, Font::CONSOLAS, 75.0f, Vec2(800.0f, 50.0f));
		break;
	case Generating:
		m_HUDMaterial.Render(m_HUDVaoID, m_HUDIndexBufferID, 8);
		m_titleRenderer.RenderText(renderer, Font::CONSOLAS, 50.0f, Vec2(800.0f, 850.0f));

		for (Features::iterator featureIter = Feature::s_featuresOnMap.begin(); featureIter != Feature::s_featuresOnMap.end(); ++featureIter){
			Feature* feature = *featureIter;
			feature->Render(renderer, true);
		}
		m_map->Render(renderer, true);
		break;
	case Playing:
		m_HUDMaterial.Render(m_HUDVaoID, m_HUDIndexBufferID, 8);

		m_statusBar.Render(renderer);
		m_messageBar.Render(renderer);

		for (Actors::iterator actorIter = Actor::s_actorsOnMap.begin(); actorIter != Actor::s_actorsOnMap.end(); ++actorIter){
			Actor* actor = actorIter->second;
			actor->Render(renderer, m_forceMapVisible);
		}
		for (Items::iterator itemIter = Item::s_itemsOnMap.begin(); itemIter != Item::s_itemsOnMap.end(); ++itemIter){
			Item* item = *itemIter;
			item->Render(renderer, m_forceMapVisible);
		}
		for (Features::iterator featureIter = Feature::s_featuresOnMap.begin(); featureIter != Feature::s_featuresOnMap.end(); ++featureIter){
			Feature* feature = *featureIter;
			feature->Render(renderer, m_forceMapVisible);
		}
		
		m_map->Render(renderer, m_forceMapVisible);
		break;
	case Defeat:
	case Victory:
		m_HUDMaterial.Render(m_HUDVaoID, m_HUDIndexBufferID, 8);
		m_titleRenderer.RenderText(renderer, Font::CONSOLAS, 100.0f, Vec2(800.0f, 450.0f));
		m_menuRenderer.RenderText(renderer, Font::CONSOLAS, 50.0f, Vec2(800.0f, 250.0f));
		break;
	}
}

///=====================================================
/// 
///=====================================================
void Game::Update(OpenGLRenderer* renderer){
	switch (m_gameState){
	case MainMenu:
		UpdateMainMenuState(renderer);
		break;
	case Quest:
		UpdateQuestState(renderer);
		break;
	case Generating:
		UpdateGeneratingState(renderer);
		break;
	case Playing:
		UpdatePlayingState(renderer);
		break;
	case Defeat:
	case Victory:
		UpdateGameOverState();
		break;
	}
}

///=====================================================
/// 
///=====================================================
void Game::UpdateMainMenuState(OpenGLRenderer* renderer){
	if (s_theInputSystem->GetKeyWentDown('Q') || s_theInputSystem->GetKeyWentDown(VK_ESCAPE)){
		m_isRunning = false;
	}
	else if (s_theInputSystem->GetKeyWentDown('N')){ //go to Quest
		SetState(Quest);
		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Quests");

		if (m_autoGenerateMap)
			m_bottomMessageRenderer.PrintText("AutoGenerate: ON", RGBAchars::GREEN);
		else
			m_bottomMessageRenderer.PrintText("AutoGenerate: OFF", RGBAchars::RED);

		m_menuRenderer.ToggleRenderFromCenter();
		m_menuRenderer.ClearText();

		const MapGeneratorRegistry* registry = MapGeneratorRegistration::GetGeneratorRegistry();
		char firstQuestLetter = 'A';

		for (MapGeneratorRegistry::const_iterator registryIter = registry->cbegin(); registryIter != registry->cend(); ++registryIter){
			std::string temp = ") ";
			temp = firstQuestLetter + temp;
			m_menuRenderer.PrintText(temp + registryIter->first);
			++firstQuestLetter;
		}
	}
	else if (s_theInputSystem->GetKeyWentDown('L')){ //load game
		LoadGame(renderer);

		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Playing");

		m_menuRenderer.ToggleRenderFromCenter();
		m_menuRenderer.ClearText();

		if (m_autoGenerateMap){
			m_bottomMessageRenderer.PrintText("AutoGenerate: ON", RGBAchars::GREEN);
		}
		else{
			m_bottomMessageRenderer.PrintText("AutoGenerate: OFF", RGBAchars::RED);
		}

		char firstQuestLetter = 'A';
		const MapGeneratorRegistry* registry = MapGeneratorRegistration::GetGeneratorRegistry();

		for (MapGeneratorRegistry::const_iterator registryIter = registry->cbegin(); registryIter != registry->cend(); ++registryIter){
			std::string temp = ") ";
			temp = firstQuestLetter + temp;
			m_menuRenderer.PrintText(temp + registryIter->first);
			++firstQuestLetter;
		}

		m_messageBar.m_isWaitingForPlayer = false;
		m_messageBar.m_text.clear();
	}
}

///=====================================================
/// 
///=====================================================
void Game::UpdateQuestState(OpenGLRenderer* renderer){
	const MapGeneratorRegistry* registry = MapGeneratorRegistration::GetGeneratorRegistry();
	char questLetter = 'A';

	for (MapGeneratorRegistry::const_iterator registryIter = registry->cbegin(); registryIter != registry->cend(); ++registryIter){
		if (s_theInputSystem->GetKeyWentDown(questLetter)){
			m_mapGenerator = (registryIter->second)->CreateMapGenerator(registryIter->first);
			if (m_mapGenerator != nullptr){
				m_map = m_mapGenerator->CreateMap(renderer);
				if (m_map != nullptr){ //go to Generating
					SetState(Generating);

					m_titleRenderer.ClearText();
					m_titleRenderer.PrintText("Generating Map");
					break;
				}
				else{
					delete m_mapGenerator;
					m_mapGenerator = nullptr;
				}
			}
		}
		++questLetter;
	}
	if (s_theInputSystem->GetKeyWentDown(VK_TAB)){
		m_autoGenerateMap = !m_autoGenerateMap;
		m_bottomMessageRenderer.ClearText();

		if (m_autoGenerateMap){
			m_bottomMessageRenderer.PrintText("AutoGenerate: ON", RGBAchars::GREEN);
		}
		else{
			m_bottomMessageRenderer.PrintText("AutoGenerate: OFF", RGBAchars::RED);
		}
	}
	else if (s_theInputSystem->GetKeyWentDown(VK_ESCAPE)){ //go back to Main Menu
		SetState(MainMenu);

		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Dagger", RGBAchars::RED);

		std::vector<std::string> unused;
		bool hasSavedGame = FindAllFilesOfType("Data/Saves/", "saveGame.xml", unused);

		m_menuRenderer.ToggleRenderFromCenter();
		m_menuRenderer.ClearText();
		m_menuRenderer.PrintText("N) New Game");
		if (hasSavedGame)
			m_menuRenderer.PrintText("L) Load Game");
		m_menuRenderer.PrintText("Q) Quit");

		m_bottomMessageRenderer.ClearText();
	}
}

///=====================================================
/// 
///=====================================================
void Game::UpdateGeneratingState(OpenGLRenderer* renderer){
	if (m_autoGenerateMap || s_theInputSystem->GetKeyWentDown(VK_RETURN)){ //go to Playing
		if (m_autoGenerateMap)
			m_mapGenerator->AutoGenerateMap(m_map);

		SetState(Playing);

		if (m_mapGenerator->m_name == "FromData"){ //load game using From Data Generator
			XMLNode root = XMLNode::parseFile("Data/Maps/saveGame.xml");
			assert(!root.isEmpty());
			XMLNode savedGame = root.getChildNode("SaveGame");
			assert(!savedGame.isEmpty());
			XMLNode entitiesNode = savedGame.getChildNode("Entities");
			assert(!entitiesNode.isEmpty());

			m_mapGenerator->FinalizeMap(m_map, entitiesNode);

			XMLNode playerNode = entitiesNode.getChildNode("Player");
			assert(!playerNode.isEmpty());
			m_player = new Player(Player(renderer), renderer, playerNode);
			m_player->AddToMap(m_map, m_player->m_position);

			ResolvePointersFromLoadGame();
		}
		else{ //any other generator
			m_mapGenerator->FinalizeMap(m_map);

			m_player = new Player(renderer);
			m_player->AddToMap(m_map, m_mapGenerator->m_playerStartPosition);
		}

		m_statusBar.m_player = m_player;

		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Playing");

		m_messageBar.m_isWaitingForPlayer = false;
		m_messageBar.m_text.clear();

		m_AISystem.m_currentStrategy = AISystem::Default;
	}
	else if (s_theInputSystem->GetKeyWentDown(VK_SPACE)){
		m_mapGenerator->ProcessOneStep(m_map);
	}
	else if (s_theInputSystem->GetKeyWentDown(VK_ESCAPE)){ //go back to Quest
		SetState(Quest);
		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Quests");

		DestroyCurrentGame(renderer);
	}
}

///=====================================================
/// 
///=====================================================
void Game::UpdatePlayingState(OpenGLRenderer* renderer){
	PROFILE_CATEGORY(Gameplay);
	if (s_theInputSystem->GetKeyWentDown(VK_ESCAPE)){ //go back to Quest
		SetState(Quest);
		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Quests");

		DestroyCurrentGame(renderer);
	}
	else if (s_theInputSystem->GetKeyWentDown('M')){ //toggle map visibility
		m_forceMapVisible = !m_forceMapVisible;
	}
	else if (s_theInputSystem->IsKeyDown('P')){ //test pathfinding
		static IntVec2 goal;
		if (m_player->m_path.HasFinishedPath()){
			Tile randomTile = m_map->GetRandomTile();
			while (randomTile.GetTileType() != TileType::Air)
				randomTile = m_map->GetRandomTile();
			goal = randomTile.m_position;
		}
		m_player->PathfindToDestination(goal, true, false);
	}
	else if (m_messageBar.m_isWaitingForPlayer){
		if (s_theInputSystem->GetKeyWentDown(VK_SPACE)){ //read more messages in the message bar
			m_messageBar.m_isWaitingForPlayer = false;
			m_messageBar.m_text.clear();
		}
	}
	else if (m_savedGame){ //return to main menu after save game
		SetState(MainMenu);

		m_player->Die();
		DestroyCurrentGame(renderer);

		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Dagger", RGBAchars::RED);

		m_menuRenderer.ClearText();
		m_menuRenderer.PrintText("N) New Game");
		m_menuRenderer.PrintText("L) Load Game");
		m_menuRenderer.PrintText("Q) Quit");
		m_menuRenderer.ToggleRenderFromCenter();

		m_bottomMessageRenderer.ClearText();

		m_savedGame = false;
	}
	else if (s_theInputSystem->GetCurrentCharacterDown() == 'S'){ //save game
		HandleDeadEntities(renderer);
		SaveGame();
	}
	///==========================================================================================================================================
	/// GAMEPLAY
	///==========================================================================================================================================
	else{
		if (Actor::s_actorsOnMap.size() == 1){ //only have a player... the following code would crash in that case
			UpdatePlayer(renderer);
			return;
		}

		//pre-compute what each AI plans to do, so the Group AI system can make adjustments before actually making moves
		Actors::const_iterator actorIter = Actor::s_actorsOnMap.cbegin();
		float maxSpeed = actorIter->first + 1.0f;
		if (!actorIter->second->IsPlayer()){
			for (; actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
				Actor* actor = actorIter->second;

				if (actor->IsPlayer()){
					break;
				}
				else{
					((NPC*)actor)->PlanNextThink();
				}
			}

			m_AISystem.PlanNextMove(*m_player);

			
			actorIter = Actor::s_actorsOnMap.cbegin();
		}

		//everyone moves
		for (; actorIter != Actor::s_actorsOnMap.cend() && actorIter->first < maxSpeed;){
			Actor* actor = actorIter->second;

			if (actor->IsPlayer()){
				bool didMove = UpdatePlayer(renderer);
				if (didMove){
					float turnOrder = actorIter->first;
					actorIter = Actor::s_actorsOnMap.erase(actorIter);
					Actor::s_actorsOnMap.emplace(++turnOrder, actor);
				}

				return;
			}
			else{
				actor->Think(true);

				float turnOrder = actorIter->first;
				actorIter = Actor::s_actorsOnMap.erase(actorIter);
				Actor::s_actorsOnMap.emplace(++turnOrder, actor);
			}
		}
	}
}

///=====================================================
/// 
///=====================================================
bool Game::UpdatePlayer(OpenGLRenderer* renderer){
	if (m_player->m_isDead){ //player died!
		SetState(Defeat);

		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Game Over!");

		m_menuRenderer.ToggleRenderFromCenter();
		m_menuRenderer.ClearText();
		m_menuRenderer.Printf("Total Turns: %i", m_player->s_turnCount);
		m_menuRenderer.Printf("Total Kills: %i", m_player->m_killCount);

		DestroyCurrentGame(renderer);

		return false;
	}

	bool isVictorious = true;
	for (std::map<float, Actor*>::const_iterator actorIter = Actor::s_actorsOnMap.cbegin(); actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
		if (m_player->GetFactionStatus(actorIter->second->m_faction->m_factionID, actorIter->second->m_ID) < 0)
			isVictorious = false;
	}

	if (isVictorious){
		SetState(Victory);

		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Victory!");

		m_menuRenderer.ToggleRenderFromCenter();
		m_menuRenderer.ClearText();
		m_menuRenderer.Printf("Total Turns: %i", m_player->s_turnCount);
		m_menuRenderer.Printf("Total Kills: %i", m_player->m_killCount);

		DestroyCurrentGame(renderer);

		return false;
	}

	if (m_player->IsReadyToThink()){
		HandleDeadEntities(renderer);
		m_player->Think(true);
		return true;
	}

	return false;
}

///=====================================================
/// 
///=====================================================
void Game::UpdateGameOverState(){
	if (s_theInputSystem->GetKeyWentDown(VK_ESCAPE)){ //go back to Main Menu
		SetState(MainMenu);
		m_titleRenderer.ClearText();
		m_titleRenderer.PrintText("Dagger", RGBAchars::RED);

		std::vector<std::string> unused;
		bool hasSavedGame = FindAllFilesOfType("Data/Saves/", "saveGame.xml", unused);

		m_menuRenderer.ClearText();
		m_menuRenderer.PrintText("N) New Game");
		if (hasSavedGame)
			m_menuRenderer.PrintText("L) Load Game");
		m_menuRenderer.PrintText("Q) Quit");

		m_bottomMessageRenderer.ClearText();

		s_theMessageBar->m_messages.clear();
	}
}

///=====================================================
/// 
///=====================================================
void Game::HandleDeadEntities(const OpenGLRenderer* renderer) const{
	//handle dead actors
	for (Actors::const_iterator deadActorIter = Actor::s_actorsOnMap.cbegin(); deadActorIter != Actor::s_actorsOnMap.cend();){
		Actor* maybeDeadActor = deadActorIter->second;
		if (maybeDeadActor->m_isDead){
			for (Actors::const_iterator reactingActorIter = Actor::s_actorsOnMap.cbegin(); reactingActorIter != Actor::s_actorsOnMap.cend(); ++reactingActorIter){
				reactingActorIter->second->ReactToDeadActor(*maybeDeadActor);
			}

			maybeDeadActor->ShutdownRendering(renderer);
			delete maybeDeadActor;
			maybeDeadActor = nullptr;
			deadActorIter = Actor::s_actorsOnMap.erase(deadActorIter);
		}
		else{
			++deadActorIter;
		}
	}
	//handle dead items
	for (Items::const_iterator deadItemIter = Item::s_itemsOnMap.cbegin(); deadItemIter != Item::s_itemsOnMap.cend();){
		Item* maybeDeadItem = *deadItemIter;
		if (maybeDeadItem->m_isDead){
			maybeDeadItem->ShutdownRendering(renderer);
			delete maybeDeadItem;
			maybeDeadItem = nullptr;
			deadItemIter = Item::s_itemsOnMap.erase(deadItemIter);
		}
		else{
			++deadItemIter;
		}
	}
	//handle dead features
	for (Features::const_iterator deadFeatureIter = Feature::s_featuresOnMap.cbegin(); deadFeatureIter != Feature::s_featuresOnMap.cend();){
		Feature* maybeDeadFeature = *deadFeatureIter;
		if (maybeDeadFeature->m_isDead){
			maybeDeadFeature->ShutdownRendering(renderer);
			delete maybeDeadFeature;
			maybeDeadFeature = nullptr;
			deadFeatureIter = Feature::s_featuresOnMap.erase(deadFeatureIter);
		}
		else{
			++deadFeatureIter;
		}
	}
}

///=====================================================
/// 
///=====================================================
void Game::DestroyCurrentGame(const OpenGLRenderer* renderer){
	m_map->DestroyMap(renderer);

	delete m_map;
	delete m_mapGenerator;
	m_map = nullptr;
	m_mapGenerator = nullptr;

	Actor::s_actorsOnMap.clear();
	Item::s_itemsOnMap.clear();
	Feature::s_featuresOnMap.clear();
}
