//=====================================================
// FromDataMapGenerator.cpp
// by Andrew Socha
//=====================================================

#include "FromDataMapGenerator.hpp"
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../Factories/NPCFactory.hpp"
#include "../Entities/NPC.hpp"
#include "../Factories/ItemFactory.hpp"
#include "../Factories/FeatureFactory.hpp"

MapGeneratorRegistration FromDataMapGenerator::s_fromDataMapGeneratorRegistration(std::string("FromData"), (MapGeneratorCreationFunc*)&CreateMapGenerator);

///=====================================================
/// 
///=====================================================
FromDataMapGenerator::FromDataMapGenerator(const std::string& name)
	:BaseMapGenerator(name){
}

///=====================================================
/// 
///=====================================================
Map* FromDataMapGenerator::CreateMap(OpenGLRenderer* renderer, const XMLNode& mapNode /*= XMLNode::emptyNode()*/){
	XMLNode saveGameNode;
	if (!mapNode.isEmpty())
		saveGameNode = mapNode;
	else{
		saveGameNode = XMLNode::parseFile("Data/Maps/saveGame.xml");
		if (!saveGameNode.isEmpty())
			saveGameNode = saveGameNode.getChildNode("SaveGame");
	}

	Map* map = new Map(saveGameNode);

	m_playerStartPosition = map->CreateMap(saveGameNode, renderer);

	return map;
}

///=====================================================
/// 
///=====================================================
void FromDataMapGenerator::ProcessOneStep(Map* /*map*/){
	//do nothing
}

///=====================================================
/// 
///=====================================================
void FromDataMapGenerator::AutoGenerateMap(Map* /*map*/){
	//do nothing
}

///=====================================================
/// 
///=====================================================
void FromDataMapGenerator::FinalizeMap(Map* map, const XMLNode& entitiesNode /*= XMLNode::emptyNode()*/){
	FATAL_ASSERT(!entitiesNode.isEmpty());

	MarkCompletelyHiddenTiles(map);
	PopulateMap(map, entitiesNode);
}

///=====================================================
/// 
///=====================================================
void FromDataMapGenerator::PopulateMap(Map* map, const XMLNode& entitiesNode /*= XMLNode::emptyNode()*/){
	for (int i = 0; i < entitiesNode.nChildNode(); ++i){
		XMLNode entityNode = entitiesNode.getChildNode(i);
		std::string entityType = entityNode.getName();

		if (entityType == "NPC"){
			std::string npcName = GetStringAttribute(entityNode, "name");
			NPCFactory* factory = NPCFactory::FindFactoryByName(npcName);

			NPC* newNPC = factory->SpawnNPC(entityNode);
			
			newNPC->AddToMap(map, newNPC->m_position);
		}
		else if (entityType == "Item"){
			std::string itemName = GetStringAttribute(entityNode, "name");
			ItemFactory* factory = ItemFactory::FindFactoryByName(itemName);

			Item* newItem = factory->SpawnItem(entityNode);

			newItem->AddToMap(map, newItem->m_position);
		}
		else if (entityType == "Feature"){
			std::string featureName = GetStringAttribute(entityNode, "name");
			FeatureFactory* factory = FeatureFactory::FindFactoryByName(featureName);

			Feature* newFeature = factory->SpawnFeature(entityNode);

			newFeature->AddToMap(map, newFeature->m_position);
		}
		else if (entityType == "Player"){
			//loaded in Game's LoadGame()
		}
		else{
			RECOVERABLE_ERROR();
		}
	}
}

