//=====================================================
// Pathfinding.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "Pathfinding.hpp"
#include "Map.hpp"
#include "Entities/Feature.hpp"

///=====================================================
/// 
///=====================================================
Path::~Path(){
	for (std::map<float, PathNode*>::const_iterator openListIter = m_openList.cbegin(); openListIter != m_openList.cend(); ++openListIter){
		delete openListIter->second;
	}
	for (std::map<IntVec2, PathNode*>::const_iterator closedListIter = m_closedList.cbegin(); closedListIter != m_closedList.cend(); ++closedListIter){
		delete closedListIter->second;
	}
}

///=====================================================
/// 
///=====================================================
void Path::Reset(){
	for (std::map<float, PathNode*>::const_iterator openListIter = m_openList.cbegin(); openListIter != m_openList.cend(); ++openListIter){
		delete openListIter->second;
	}
	for (std::map<IntVec2, PathNode*>::const_iterator closedListIter = m_closedList.cbegin(); closedListIter != m_closedList.cend(); ++closedListIter){
		delete closedListIter->second;
	}
	m_openList.clear();
	m_closedList.clear();
	m_path.clear();
	m_goal = IntVec2(-1, -1);
}

///=====================================================
/// 
///=====================================================
const std::deque<PathNode*>& Path::CalcPath(bool onlyVisibleTiles, const Actor* actor, const IntVec2& start, const IntVec2& goal, Map* map, bool doActorsBlock /*= false*/, bool calcFullPath /*= true*/, bool doFeaturesBlock /*= true*/){
	FATAL_ASSERT(map != nullptr);

	if (m_goal != goal){
		for (std::map<float, PathNode*>::const_iterator openListIter = m_openList.cbegin(); openListIter != m_openList.cend(); ++openListIter){
			delete openListIter->second;
		}
		for (std::map<IntVec2, PathNode*>::const_iterator closedListIter = m_closedList.cbegin(); closedListIter != m_closedList.cend(); ++closedListIter){
			delete closedListIter->second;
		}
		m_openList.clear();
		m_closedList.clear();

		m_goal = goal;
	}

	if (m_openList.empty()){
		if (m_closedList.empty()){ //first step to calculate this path
			PathNode* firstNode = new PathNode();
			firstNode->m_parent = nullptr;
			firstNode->m_position = start;

			if (start == goal){
				m_path.clear();

				m_closedList[firstNode->m_position] = firstNode;
				m_path.push_front(firstNode);
				return m_path;
			}

			firstNode->g = 0.0f;
			firstNode->f = (float)CalcManhattanDistance(start, goal);
			m_openList.emplace(firstNode->f, firstNode);
		}
		else{
			return m_path; //already finished calculating this path
		}
	}
	
	m_path.clear();


	while (!m_openList.empty()){
		PathNode* activeNode = m_openList.begin()->second;

		if (activeNode->m_position == goal){ //reached goal
			m_closedList[activeNode->m_position] = activeNode;
			m_openList.erase(m_openList.begin());

			while (activeNode->m_parent != nullptr){ //iterate through parents to create path
				m_path.push_front(activeNode);
				activeNode = activeNode->m_parent;
			}
			m_path.push_front(activeNode);

			for (std::map<float, PathNode*>::const_iterator openListIter = m_openList.cbegin(); openListIter != m_openList.cend(); ++openListIter){
				delete openListIter->second;
			}
			m_openList.clear();

			return m_path;
		}

		m_openList.erase(m_openList.begin());

		const std::vector<Tile*>& adjacentTiles = (actor != nullptr && onlyVisibleTiles) ? map->GetVisibleTilesWithinRadius(*actor, activeNode->m_position, TileType::Air) : map->GetTilesWithinRadius(activeNode->m_position, TileType::Air);
		for (std::vector<Tile*>::const_iterator tileIter = adjacentTiles.cbegin(); tileIter != adjacentTiles.cend(); ++tileIter){
			const Tile* const& tile = *tileIter;
			if (m_closedList.find(tile->m_position) != m_closedList.end()) continue; //already in closed list
			if (doActorsBlock && tile->m_actor != nullptr && tile->m_position != m_goal) continue; //actor blocks movement
			if (doFeaturesBlock && tile->m_feature != nullptr && ((tile->m_feature->m_blocksMovementUsed && tile->m_feature->m_isUsed)
					|| (tile->m_feature->m_blocksMovementUnused && !tile->m_feature->m_isUsed))) continue; //feature blocks movement
			if (tile->m_position == activeNode->m_position) continue; //ignore the active tile
			
			for (std::map<float, PathNode*>::iterator openListIter = m_openList.begin(); openListIter != m_openList.end(); ++openListIter){
				PathNode* openListNode = openListIter->second;
				if (openListNode->m_position == tile->m_position){ //already in open list
					float g = CalcDistance(activeNode->m_position, openListNode->m_position) + activeNode->g;
					if (g < openListNode->g){
						float h = (float)CalcManhattanDistance(openListNode->m_position, goal);
						openListNode->g = g;
						openListNode->f = g + h;
						openListNode->m_parent = activeNode;

						//move this node to its new location in the open list
						m_openList.erase(openListIter);
						m_openList.emplace(openListNode->f, openListNode);
					}
					goto skipToNextAdjacentTile;
				}
			}

			//not found in open list... so add it
			PathNode* adjacentNode = new PathNode();
			adjacentNode->m_parent = activeNode;
			adjacentNode->m_position = tile->m_position;
			adjacentNode->g = CalcDistance(adjacentNode->m_position, activeNode->m_position) + activeNode->g;
			float h = (float)CalcManhattanDistance(adjacentNode->m_position, goal);
			adjacentNode->f = adjacentNode->g + h;

			m_openList.emplace(adjacentNode->f, adjacentNode);

skipToNextAdjacentTile:;
		}

		m_closedList[activeNode->m_position] = activeNode;

		if (!calcFullPath){
			while (activeNode->m_parent != nullptr){ //iterate through parents to create path
				m_path.push_front(activeNode);
				activeNode = activeNode->m_parent;
			}
			m_path.push_front(activeNode);
			return m_path;
		}
	}

	return m_path; //return an empty path, as none exists
}