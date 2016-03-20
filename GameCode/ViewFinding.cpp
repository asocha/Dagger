//=====================================================
// ViewFinding.cpp
// by Andrew Socha
//=====================================================

#include "ViewFinding.hpp"
#include "Map.hpp"
#include "Entities/Feature.hpp"

///=====================================================
/// Determine what tiles are visible by doing many raycasts
///=====================================================
std::set<Tile*>& CalculateFieldOfView(Map* map, const IntVec2& playerPosition){
	FATAL_ASSERT(map != nullptr);

	static std::set<Tile*> visibleTiles;
	visibleTiles.clear();

	Tile& playerTile = map->GetTileAtPosition(playerPosition);

	//use the edge of the map as the end of the raycast, starting at the player's position
	for (int h = 0; h < map->m_size.y; ++h){
		Tile& tile = map->m_tiles[h * map->m_size.x];
		const std::vector<Tile*>& tiles = CalculateLineOfSight(map, playerTile, tile, true);
		for (std::vector<Tile*>::const_iterator tileIter = tiles.cbegin(); tileIter != tiles.cend(); ++tileIter){
			visibleTiles.insert(*tileIter);
		}

		const std::vector<Tile*>& tiles2 = CalculateLineOfSight(map, playerTile, tile, false);
		for (std::vector<Tile*>::const_iterator tileIter = tiles2.cbegin(); tileIter != tiles2.cend(); ++tileIter){
			visibleTiles.insert(*tileIter);
		}


		Tile& tile2 = map->m_tiles[map->m_size.x - 1 + h * map->m_size.x];
		const std::vector<Tile*>& tiles3 = CalculateLineOfSight(map, playerTile, tile2, true);
		for (std::vector<Tile*>::const_iterator tileIter = tiles3.cbegin(); tileIter != tiles3.cend(); ++tileIter){
			visibleTiles.insert(*tileIter);
		}

		const std::vector<Tile*>& tiles4 = CalculateLineOfSight(map, playerTile, tile2, false);
		for (std::vector<Tile*>::const_iterator tileIter = tiles4.cbegin(); tileIter != tiles4.cend(); ++tileIter){
			visibleTiles.insert(*tileIter);
		}
	}
	for (int w = 0; w < map->m_size.x; ++w){
		Tile& tile = map->m_tiles[w];
		const std::vector<Tile*>& tiles = CalculateLineOfSight(map, playerTile, tile, true);
		for (std::vector<Tile*>::const_iterator tileIter = tiles.cbegin(); tileIter != tiles.cend(); ++tileIter){
			visibleTiles.insert(*tileIter);
		}

		const std::vector<Tile*>& tiles2 = CalculateLineOfSight(map, playerTile, tile, false);
		for (std::vector<Tile*>::const_iterator tileIter = tiles2.cbegin(); tileIter != tiles2.cend(); ++tileIter){
			visibleTiles.insert(*tileIter);
		}


		Tile& tile2 = map->m_tiles[w + (map->m_size.y - 1) * map->m_size.x];
		const std::vector<Tile*>& tiles3 = CalculateLineOfSight(map, playerTile, tile2, true);
		for (std::vector<Tile*>::const_iterator tileIter = tiles3.cbegin(); tileIter != tiles3.cend(); ++tileIter){
			visibleTiles.insert(*tileIter);
		}

		const std::vector<Tile*>& tiles4 = CalculateLineOfSight(map, playerTile, tile2, false);
		for (std::vector<Tile*>::const_iterator tileIter = tiles4.cbegin(); tileIter != tiles4.cend(); ++tileIter){
			visibleTiles.insert(*tileIter);
		}
	}

	return visibleTiles;
}

///=====================================================
/// Determine what tiles are visible in a line by using a raycast
///=====================================================
std::vector<Tile*>& CalculateLineOfSight(Map* map, const Tile& startTile, const Tile& endTile, bool prioritizeX){
	FATAL_ASSERT(map != nullptr);

	static std::vector<Tile*> visibleTiles;
	visibleTiles.clear();

	IntVec2 rayDisplacement = endTile.m_position - startTile.m_position;
	if (rayDisplacement.x == 0 && rayDisplacement.y == 0) return visibleTiles;

	float inverseRayDisplaymentX = 1.0f / (float)rayDisplacement.x;
	float inverseRayDisplaymentY = 1.0f / (float)rayDisplacement.y;

	float tDeltaX = abs(inverseRayDisplaymentX);
	float tDeltaY = abs(inverseRayDisplaymentY);

	int tileStepX = inverseRayDisplaymentX >= 0.0f ? 1 : -1;
	int tileStepY = inverseRayDisplaymentY >= 0.0f ? 1 : -1;

	float tToNextIntersectionX = 0.0f;
	float tToNextIntersectionY = 0.0f;
	float tToNextIntersection = 0.0f;

	if (rayDisplacement.x == 0)
		tToNextIntersectionX = 10000.0f;
	else if (rayDisplacement.y == 0)
		tToNextIntersectionY = 10000.0f;

	IntVec2 impactTile = startTile.m_position;

	for (float t = 0.0f; t <= 1.0f; t += tToNextIntersection){
		Tile& tile = map->GetTileAtPosition(impactTile);
		visibleTiles.push_back(&tile);

		if (tile.GetTileType() != TileType::Air){
			return visibleTiles;
		}

		if (tile.m_feature != nullptr && ((tile.m_feature->m_blocksVisibilityUnused == true && !tile.m_feature->m_isUsed) || (tile.m_feature->m_blocksVisibilityUsed == true && tile.m_feature->m_isUsed))){
			return visibleTiles;
		}

		if (tToNextIntersectionX < tToNextIntersectionY || (prioritizeX && (tToNextIntersectionX == tToNextIntersectionY))){ //a really simple, inefficient way to use both ways in the case of a tie
			tToNextIntersection = tToNextIntersectionX;
			tToNextIntersectionX = tDeltaX;
			tToNextIntersectionY -= tToNextIntersection;
			impactTile.x += tileStepX;
		}
		else{
			tToNextIntersection = tToNextIntersectionY;
			tToNextIntersectionX -= tToNextIntersection;
			tToNextIntersectionY = tDeltaY;
			impactTile.y += tileStepY;
		}
	}

	return visibleTiles;
}
