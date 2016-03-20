//=====================================================
// CellularAutomatonMapGenerator.cpp
// by Andrew Socha
//=====================================================

#include "CellularAutomatonMapGenerator.hpp"
#include "../Map.hpp"

MapGeneratorRegistration CellularAutomatonMapGenerator::s_cellularAutomatonMapGeneratorRegistration(std::string("CellularAutomaton"), (MapGeneratorCreationFunc*)&CreateMapGenerator);

///=====================================================
/// 
///=====================================================
CellularAutomatonMapGenerator::CellularAutomatonMapGenerator(const std::string& name)
:BaseMapGenerator(name),
m_iterationCount(0){
}

///=====================================================
/// 
///=====================================================
Map* CellularAutomatonMapGenerator::CreateMap(OpenGLRenderer* renderer, const XMLNode& /*mapNode*/ /*= XMLNode::emptyNode()*/){
	Map* map = new Map(25);

	map->CreateMap(renderer);

	for (Tiles::iterator tileIter = map->m_tiles.begin(); tileIter != map->m_tiles.end(); ++tileIter){
		if (GetRandomFloatZeroToOne() < 0.4f)
			tileIter->SetTileType(TileType::Wall);
	}

	m_iterationCount = 0;

	return map;
}

///=====================================================
/// if walls w/in radius 1 >= 5 set wall
/// if walls w/in radius 2 <= 2 set wall
/// else set air
///=====================================================
void CellularAutomatonMapGenerator::ProcessOneStep(Map* map){
	FATAL_ASSERT(map != nullptr);

	++m_iterationCount;

	static std::vector<Tile*> tilesToConvertToWalls;
	static std::vector<Tile*> tilesToConvertToAir;
	tilesToConvertToWalls.clear();
	tilesToConvertToAir.clear();

	for (Tiles::iterator tileIter = map->m_tiles.begin(); tileIter != map->m_tiles.end(); ++tileIter){
		int numAdjacentWalls = 0;
		std::vector<Tile*>& adjacentTiles = map->GetTilesWithinRadius(tileIter->m_position);
		for (std::vector<Tile*>::const_iterator adjacentTileIter = adjacentTiles.cbegin(); adjacentTileIter != adjacentTiles.cend(); ++adjacentTileIter){
			if ((*adjacentTileIter)->GetTileType() == TileType::Wall)
				++numAdjacentWalls;
		}

		if (numAdjacentWalls >= 5){
			if (tileIter->GetTileType() == TileType::Air)
				tilesToConvertToWalls.push_back(&*tileIter);
			continue;
		}

		if (m_iterationCount > 10)
			continue;


		numAdjacentWalls = 0;
		std::vector<Tile*>& twoAdjacentTiles = map->GetTilesWithinRadius(tileIter->m_position, 2);
		for (std::vector<Tile*>::const_iterator twoAdjacentTileIter = twoAdjacentTiles.cbegin(); twoAdjacentTileIter != twoAdjacentTiles.cend(); ++twoAdjacentTileIter){
			if ((*twoAdjacentTileIter)->GetTileType() == TileType::Wall)
				++numAdjacentWalls;
		}

		if (numAdjacentWalls <= 2){
			if (tileIter->GetTileType() == TileType::Air)
				tilesToConvertToWalls.push_back(&*tileIter);
			continue;
		}

		if (tileIter->GetTileType() == TileType::Wall){
			tilesToConvertToAir.push_back(&*tileIter);
		}
	}

	for (std::vector<Tile*>::const_iterator tileIter = tilesToConvertToWalls.cbegin(); tileIter != tilesToConvertToWalls.cend(); ++tileIter){
		(*tileIter)->SetTileType(TileType::Wall);
	}
	for (std::vector<Tile*>::const_iterator tileIter = tilesToConvertToAir.cbegin(); tileIter != tilesToConvertToAir.cend(); ++tileIter){
		(*tileIter)->SetTileType(TileType::Air);
	}
}

///=====================================================
/// 
///=====================================================
void CellularAutomatonMapGenerator::AutoGenerateMap(Map* map){
	RECOVERABLE_ASSERT(map != nullptr);
	for (int i = 0; i < 12; ++i)
		ProcessOneStep(map);
}
