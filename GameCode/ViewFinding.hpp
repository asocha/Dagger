//=====================================================
// ViewFinding.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_ViewFinding__
#define __included_ViewFinding__

class Map;
struct IntVec2;
class Tile;
#include <vector>
#include <set>


std::set<Tile*>& CalculateFieldOfView(Map* map, const IntVec2& playerPosition);
std::vector<Tile*>& CalculateLineOfSight(Map* map, const Tile& startTile, const Tile& endTile, bool prioritizeX);



#endif