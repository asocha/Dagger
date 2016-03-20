//=====================================================
// BaseMapGenerator.cpp
// by Andrew Socha
//=====================================================

#include "BaseMapGenerator.hpp"
#include "../Map.hpp"
#include "../Factories/NPCFactory.hpp"
#include "../Entities/NPC.hpp"
#include "../Factories/ItemFactory.hpp"
#include <assert.h>
#include "Engine/Core/StringTable.hpp"

MapGeneratorRegistry* MapGeneratorRegistration::s_registry = nullptr;

///=====================================================
/// 
///=====================================================
void BaseMapGenerator::PopulateMap(Map* map, const XMLNode& /*entitiesNode*/ /*= XMLNode::emptyNode()*/){
	assert(map != nullptr);

	//guarantee an orc shaman
	NPCFactory* orcShamanFactory = NPCFactory::FindFactoryByName("Orc Shaman");
	assert(orcShamanFactory != nullptr);
	NPC* newNPC = orcShamanFactory->SpawnNPC();

	std::vector<Tile*>& airTiles = map->GetAllTilesOfType(TileType::Air, true);
	assert(!airTiles.empty());
	int index = GetRandomIntLessThan(airTiles.size());
	newNPC->AddToMap(map, airTiles.at(index)->m_position);
	airTiles.erase(airTiles.begin() + index);

	//guarantee a paladin
	NPCFactory* paladinFactory = NPCFactory::FindFactoryByName("Zombie Paladin");
	assert(paladinFactory != nullptr);
	newNPC = paladinFactory->SpawnNPC();

	assert(!airTiles.empty());
	index = GetRandomIntLessThan(airTiles.size());
	newNPC->AddToMap(map, airTiles.at(index)->m_position);
	airTiles.erase(airTiles.begin() + index);

	//guarantee a murloc
	NPCFactory* murlocFactory = NPCFactory::FindFactoryByName("Murloc");
	assert(murlocFactory != nullptr);
	newNPC = murlocFactory->SpawnNPC();

	assert(!airTiles.empty());
	index = GetRandomIntLessThan(airTiles.size());
	newNPC->AddToMap(map, airTiles.at(index)->m_position);
	airTiles.erase(airTiles.begin() + index);

	//guarantee 3 apprentices
	for (int i = 0; i < 3; ++i){
		NPCFactory* apprenticeFactory = NPCFactory::FindFactoryByName("Apprentice");
		assert(apprenticeFactory != nullptr);
		newNPC = apprenticeFactory->SpawnNPC();

		assert(!airTiles.empty());
		index = GetRandomIntLessThan(airTiles.size());
		newNPC->AddToMap(map, airTiles.at(index)->m_position);
		airTiles.erase(airTiles.begin() + index);
	}

	//NPCs
	for (size_t i = 0; i < map->m_tiles.size(); i += 150){
		NPCFactoryRegistry::const_iterator factoryIter = NPCFactory::s_factories.cbegin();
		int factoryNum = GetRandomIntLessThan(NPCFactory::s_factories.size());
		for (int factoryIndex = 0; factoryIndex < factoryNum; ++factoryIndex)
			++factoryIter;

		if (GetStringID("Murloc") == factoryIter->first) //don't make any more murlocs
			continue;

		newNPC = factoryIter->second->SpawnNPC();

		assert(!airTiles.empty());
		index = GetRandomIntLessThan(airTiles.size());
		newNPC->AddToMap(map, airTiles.at(index)->m_position);
		airTiles.erase(airTiles.begin() + index);
	}

	//items
	for (size_t i = 0; i < map->m_tiles.size(); i += 150){
		ItemFactoryRegistry::const_iterator factoryIter = ItemFactory::s_factories.cbegin();
		int factoryNum = GetRandomIntLessThan(ItemFactory::s_factories.size());
		for (int factoryIndex = 0; factoryIndex < factoryNum; ++factoryIndex)
			++factoryIter;

		Item* newItem = factoryIter->second->SpawnItem();

		assert(!airTiles.empty());
		index = GetRandomIntLessThan(airTiles.size());
		newItem->AddToMap(map, airTiles.at(index)->m_position);
		airTiles.erase(airTiles.begin() + index);
	}
}

///=====================================================
/// 
///=====================================================
void BaseMapGenerator::SurroundMapWithWalls(Map* map){
	assert(map != nullptr);

	for (int h = 0; h < map->m_size.y; ++h){
		map->m_tiles[h * map->m_size.x].SetTileType(TileType::Wall);
		map->m_tiles[map->m_size.x - 1 + h * map->m_size.x].SetTileType(TileType::Wall);
	}
	for (int w = 0; w < map->m_size.x; ++w){
		map->m_tiles[w].SetTileType(TileType::Wall);
		map->m_tiles[w + (map->m_size.y - 1) * map->m_size.x].SetTileType(TileType::Wall);
	}
}

///=====================================================
/// 
///=====================================================
void BaseMapGenerator::MarkCompletelyHiddenTiles(Map* map){
	assert(map != nullptr);

	for (std::vector<Tile>::iterator tileIter = map->m_tiles.begin(); tileIter != map->m_tiles.end(); ++tileIter){
		const Tile& northTile = map->GetTileAtPosition(tileIter->m_position + IntVec2(0, 1));
		if (northTile.GetTileType() == TileType::Air) continue;

		const Tile& southTile = map->GetTileAtPosition(tileIter->m_position + IntVec2(0, -1));
		if (southTile.GetTileType() == TileType::Air) continue;

		const Tile& eastTile = map->GetTileAtPosition(tileIter->m_position + IntVec2(1, 0));
		if (eastTile.GetTileType() == TileType::Air) continue;

		const Tile& westTile = map->GetTileAtPosition(tileIter->m_position + IntVec2(-1, 0));
		if (westTile.GetTileType() == TileType::Air) continue;

		tileIter->m_isCompletelyHidden = true;
		tileIter->m_isVisibleToPlayer = false;
		tileIter->m_hasBeenSeen = false;
	}
}

///=====================================================
/// 
///=====================================================
void BaseMapGenerator::FinalizeMap(Map* map, const XMLNode& /*entitiesNode*/ /*= XMLNode::emptyNode()*/){
	assert(map != nullptr);

	SurroundMapWithWalls(map);

	MarkCompletelyHiddenTiles(map);
	

	//set player start position
	while (m_playerStartPosition.x == -1){
		Tile& tile = map->GetRandomTile();
		if (tile.CanBeMovedInto()){
			m_playerStartPosition = tile.m_position;
		}
	}

	PopulateMap(map);
}
