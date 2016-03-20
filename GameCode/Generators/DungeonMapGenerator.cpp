//=====================================================
// DungeonMapGenerator.cpp
// by Andrew Socha
//=====================================================

#include "DungeonMapGenerator.hpp"
#include "../Map.hpp"
#include "../Factories/FeatureFactory.hpp"

MapGeneratorRegistration DungeonMapGenerator::s_dungeonMapGeneratorRegistration(std::string("Dungeon"), (MapGeneratorCreationFunc*)&CreateMapGenerator);

///=====================================================
/// 
///=====================================================
DungeonMapGenerator::DungeonMapGenerator(const std::string& name)
	:BaseMapGenerator(name){
}

///=====================================================
/// 
///=====================================================
Map* DungeonMapGenerator::CreateMap(OpenGLRenderer* renderer, const XMLNode& /*mapNode*/ /*= XMLNode::emptyNode()*/){
	Map* map = new Map(25);

	map->CreateMap(renderer);

	for (Tiles::iterator tileIter = map->m_tiles.begin(); tileIter != map->m_tiles.end(); ++tileIter){
		tileIter->SetTileType(TileType::Wall);
	}

	//set the middle square to air

	Tile& tile = map->GetTileAtPosition(map->m_size / 2);
	tile.SetTileType(TileType::Air);
	

	return map;
}

///=====================================================
/// 
///=====================================================
void DungeonMapGenerator::ProcessOneStep(Map* map){
	FATAL_ASSERT(map != nullptr);
	int numAttempts = 0;

	const std::vector<Tile*>& airTiles = map->GetAllTilesOfType(TileType::Air);
	if (airTiles.empty())
		return;

restart:
	++numAttempts;
	if (numAttempts > 1000) //give up trying to do anything this iteration
		return;

	std::vector<Tile*> tilesToConvertToAir;


	Tile* tile = airTiles.at(GetRandomIntLessThan(airTiles.size()));

	//make a hallway

	int xIncrement = GetRandomIntInRange(-1, 1);
	int yIncrement = GetRandomIntInRange(-1, 1);
	while (yIncrement == 0 && xIncrement == 0){
		xIncrement = GetRandomIntInRange(-1, 1);
		yIncrement = GetRandomIntInRange(-1, 1);
	}

	int hallLength;
	if (yIncrement == 0)
		hallLength = GetRandomIntInRange(10, 25);
	else{
		if (xIncrement != 0)
			hallLength = GetRandomIntInRange(15, 30); //diagonal halls have to be ~2x as long to alternate between going vertical and horizontal
		else
			hallLength = GetRandomIntInRange(8, 15);
	}

	IntVec2 tilePos = tile->m_position;
	IntVec2 doorPos = tilePos;
	bool incrementXNext = true;
	for (int i = 0; i < hallLength; ++i){
		doorPos = tilePos;

		if (yIncrement == 0 || (xIncrement != 0 && incrementXNext))
			tilePos.x += xIncrement;
		else
			tilePos.y += yIncrement;

		Tile& currentTile = map->GetTileAtPosition(tilePos);
		if (currentTile.GetTileType() == TileType::Air){
			break;
		}
		if (currentTile.GetTileType() == TileType::OutsideOfWorld){
			goto restart;
		}
		if ((tilePos.x == 0) || (tilePos.x == map->m_size.x - 1) || (tilePos.y == 0) || (tilePos.y == map->m_size.y - 1)){ //edge of world
			goto restart;
		}
		
		tilesToConvertToAir.push_back(&currentTile);

		incrementXNext = !incrementXNext;
	}

	if (tilesToConvertToAir.size() <= 2) //don't make a really short hall
		goto restart;

	//make a room if we can

	Tile& startTile = map->GetTileAtPosition(tilePos);

	if (startTile.GetTileType() == TileType::Wall){
		int roomSizeX = GetRandomIntInRange(5, 10);
		int roomSizeY = GetRandomIntInRange(3, 5);
		
		int roomStartX = GetRandomIntInRange(0, roomSizeX - 1);
		int roomStartY = GetRandomIntInRange(0, roomSizeY - 1);

		if (xIncrement > 0)
			doorPos.x -= roomStartX;
		else if (xIncrement < 0)
			doorPos.x += (roomSizeX - roomStartX) - 1;
		if (yIncrement > 0)
			doorPos.y -= roomStartY;
		else if (yIncrement < 0)
			doorPos.y += (roomSizeY - roomStartY) - 1;

		tilePos -= IntVec2(roomStartX, roomStartY);
		int tilePosYStart = tilePos.y;
		for (int x = 0; x < roomSizeX; ++x){
			for (int y = 0; y < roomSizeY; ++y){
				const Tile& currentTile = map->GetTileAtPosition(tilePos);
				if ((tilePos.x == 0) || (tilePos.x == map->m_size.x - 1) || (tilePos.y == 0) || (tilePos.y == map->m_size.y - 1)){ //edge of world
					if (y >= 3){
						roomSizeY = y;
						break;
					}
					if (x >= 5)
						goto makeDoor;
					goto restart;
				}
				if (currentTile.GetTileType() == TileType::Air)
					goto restart;
				if (currentTile.GetTileType() == TileType::OutsideOfWorld){
					goto restart;
				}

				tilesToConvertToAir.push_back((Tile*)&currentTile);
				++tilePos.y;
			}
			++tilePos.x;
			tilePos.y = tilePosYStart;
		}

makeDoor:
		//make a door
		if ((xIncrement == 0 || yIncrement == 0) && map->GetTileAtPosition(doorPos).m_feature == nullptr){
			FeatureFactory* doorFactory = FeatureFactory::FindFactoryByType(Feature::Door);
			if (doorFactory == nullptr){
				RECOVERABLE_ERROR();
				goto finishStep;
			}

			Feature* door = doorFactory->SpawnFeature();
			door->AddToMap(map, doorPos);
		}
	}
	else if (GetRandomIntLessThan(5) > 0) //80% chance to restart instead of making a room-less hallway
		goto restart;

finishStep:
	//actually change the tiles to air
	for (std::vector<Tile*>::const_iterator tileIter = tilesToConvertToAir.cbegin(); tileIter != tilesToConvertToAir.cend(); ++tileIter){
		(*tileIter)->SetTileType(TileType::Air);
	}
}

///=====================================================
/// 
///=====================================================
void DungeonMapGenerator::AutoGenerateMap(Map* map){
	RECOVERABLE_ASSERT(map != nullptr);
	for (int i = 0; i < 25; ++i)
		ProcessOneStep(map);
}
