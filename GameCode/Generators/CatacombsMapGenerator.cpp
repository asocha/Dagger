//=====================================================
// CatacombsMapGenerator.cpp
// by Andrew Socha
//=====================================================

#include "CatacombsMapGenerator.hpp"
#include "../Map.hpp"
#include "../Factories/ItemFactory.hpp"
#include "../Factories/NPCFactory.hpp"
#include "../Pathfinding.hpp"
#include "../Entities/NPC.hpp"

MapGeneratorRegistration CatacombsMapGenerator::s_catacombsMapGeneratorRegistration(std::string("Catacombs"), (MapGeneratorCreationFunc*)&CreateMapGenerator);

///=====================================================
/// 
///=====================================================
CatacombsMapGenerator::CatacombsMapGenerator(const std::string& name):
BaseMapGenerator(name),
m_currentStep(0),
m_vampireHallway(-1, -1),
m_entranceHallway(-1, -1){
}

///=====================================================
/// 
///=====================================================
Map* CatacombsMapGenerator::CreateMap(OpenGLRenderer* renderer, const XMLNode& /*mapNode*/ /*= XMLNode::emptyNode()*/){
	Map* map = new Map(IntVec2(64,32));

	map->CreateMap(renderer);

	for (Tiles::iterator tileIter = map->m_tiles.begin(); tileIter != map->m_tiles.end(); ++tileIter){
		tileIter->SetTileType(TileType::Wall);
	}

	return map;
}

///=====================================================
/// 
///=====================================================
void CatacombsMapGenerator::ProcessOneStep(Map* map){
	FATAL_ASSERT(map != nullptr);

	Tile* tile;
	IntVec2 hallwayPos, roomStartPos, currentPos;
	int height, width;
	Path validationPath;

	switch (m_currentStep)
	{
	case 0:
		for (int x = 5; x <= 59; ++x){
			for (int y = 2; y <= 30; ++y){
				if (GetRandomFloatZeroToOne() >= 0.66f)
					continue;
				Tile& airTile = map->GetTileAtPosition(IntVec2(x, y));
				airTile.SetTileType(TileType::Air);
			}
		}
		++m_currentStep;
		break;
	case 1:
		width = GetRandomIntInRange(3, 4);
		height = GetRandomIntInRange(4, 6);

		roomStartPos = IntVec2(map->m_size.x - 2,(map->m_size.y / 2) - (height / 2));
		currentPos = roomStartPos + IntVec2(1, -1);

		//surround with walls
		for (int x = -1; x < width + 1; ++x){
			for (int y = -1; y < height + 1; ++y){
				Tile& wallTile = map->GetTileAtPosition(currentPos);
				wallTile.SetTileType(TileType::Wall);

				++currentPos.y;
			}
			--currentPos.x;
			currentPos.y = roomStartPos.y - 1;
		}

		currentPos = roomStartPos;

		//make room
		for (int x = 0; x < width; ++x){
			for (int y = 0; y < height; ++y){
				Tile& roomTile = map->GetTileAtPosition(currentPos);
				roomTile.SetTileType(TileType::Air);

				++currentPos.y;
			}
			--currentPos.x;
			currentPos.y = roomStartPos.y;
		}

		m_vampireHallway = IntVec2(currentPos.x + 1, currentPos.y + (height / 2));

		hallwayPos = IntVec2(m_vampireHallway.x - 1, m_vampireHallway.y);
		currentPos = hallwayPos;

		tile = &map->GetTileAtPosition(currentPos);
		while (tile->m_tileType != TileType::Air){
			tile->SetTileType(TileType::Air);

			--currentPos.x;
			tile = &map->GetTileAtPosition(currentPos);
		}

		++m_currentStep;
		break;
	case 2:
		width = GetRandomIntInRange(3, 4);
		height = GetRandomIntInRange(4, 6);

		roomStartPos = IntVec2(1, (map->m_size.y / 2) - (height / 2));
		currentPos = roomStartPos + IntVec2(-1, -1);

		//surround with walls
		for (int x = -1; x < width + 1; ++x){
			for (int y = -1; y < height + 1; ++y){
				Tile& wallTile = map->GetTileAtPosition(currentPos);
				wallTile.SetTileType(TileType::Wall);

				++currentPos.y;
			}
			++currentPos.x;
			currentPos.y = roomStartPos.y - 1;
		}

		currentPos = roomStartPos;

		//make room
		for (int x = 0; x < width; ++x){
			for (int y = 0; y < height; ++y){
				Tile& roomTile = map->GetTileAtPosition(currentPos);
				roomTile.SetTileType(TileType::Air);

				++currentPos.y;
			}
			++currentPos.x;
			currentPos.y = roomStartPos.y;
		}

		m_entranceHallway = IntVec2(currentPos.x - 1, currentPos.y + (height / 2));

		hallwayPos = IntVec2(m_entranceHallway.x + 1, m_entranceHallway.y);
		currentPos = hallwayPos;
		tile = &map->GetTileAtPosition(currentPos);
		while (tile->m_tileType != TileType::Air){
			tile->SetTileType(TileType::Air);

			++currentPos.x;
			tile = &map->GetTileAtPosition(currentPos);
		}

		++m_currentStep;
		break;
	case 3:
		validationPath.CalcPath(false, nullptr, m_entranceHallway, m_vampireHallway, map);

		if (validationPath.m_path.empty()){ //invalid map
			m_currentStep = 0;
			for (Tiles::iterator tileIter = map->m_tiles.begin(); tileIter != map->m_tiles.end(); ++tileIter){
				tileIter->SetTileType(TileType::Wall);
			}

			break;
		}

		++m_currentStep;
		break;
	default:
		break;
	}
}

///=====================================================
/// 
///=====================================================
void CatacombsMapGenerator::PopulateMap(Map* map, const XMLNode& /*entitiesNode*/ /*= XMLNode::emptyNode()*/){
	FATAL_ASSERT(map != nullptr);

	NPCFactory* skeletonFactory = NPCFactory::FindFactoryByName("Skeleton");
	FATAL_ASSERT(skeletonFactory != nullptr);

	NPC* newSkeleton = skeletonFactory->SpawnNPC();
	newSkeleton->AddToMap(map, m_entranceHallway + IntVec2(-1, 1));

	newSkeleton = skeletonFactory->SpawnNPC();
	newSkeleton->AddToMap(map, m_entranceHallway + IntVec2(-1, -1));


	NPCFactory* vampireFactory = NPCFactory::FindFactoryByName("Vampire Lord");
	FATAL_ASSERT(vampireFactory != nullptr);

	NPC* newVampire = vampireFactory->SpawnNPC();
	newVampire->AddToMap(map, m_vampireHallway + IntVec2(2, 1));


	NPCFactory* paladinFactory = NPCFactory::FindFactoryByName("Paladin");
	FATAL_ASSERT(paladinFactory != nullptr);

	NPC* newPaladin = paladinFactory->SpawnNPC();
	newPaladin->AddToMap(map, m_entranceHallway + IntVec2(-2, 0));

	m_playerStartPosition = m_entranceHallway + IntVec2(-2, -1);

	//skeletons
	for (int x = 5; x <= 59; ++x){
		for (int y = 2; y <= 30; ++y){
			const Tile& tile = map->GetTileAtPosition(IntVec2(x, y));
			if (tile.m_tileType != TileType::Air)
				continue;
			if (GetRandomFloatZeroToOne() >= 0.02f)
				continue;

			newSkeleton = skeletonFactory->SpawnNPC();
			newSkeleton->AddToMap(map, tile.m_position);
		}
	}

	//items
	for (unsigned int i = 0; i < map->m_tiles.size(); i += 250){
		ItemFactoryRegistry::const_iterator factoryIter = ItemFactory::s_factories.cbegin();
		int factoryNum = GetRandomIntLessThan(ItemFactory::s_factories.size());
		for (int factoryIndex = 0; factoryIndex < factoryNum; ++factoryIndex)
			++factoryIter;

		Item* newItem = factoryIter->second->SpawnItem();

		const std::vector<Tile*>& airTiles = map->GetAllTilesOfType(TileType::Air, true);
		FATAL_ASSERT(!airTiles.empty());
		newItem->AddToMap(map, airTiles.at(GetRandomIntLessThan(airTiles.size()))->m_position);
	}
}

///=====================================================
/// 
///=====================================================
void CatacombsMapGenerator::AutoGenerateMap(Map* map){
	RECOVERABLE_ASSERT(map != nullptr);
	while (m_currentStep <= 3){
		ProcessOneStep(map);
	}
}