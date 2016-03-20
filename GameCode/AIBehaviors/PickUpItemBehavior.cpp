//=====================================================
// PickUpItemBehavior.cpp
// by Andrew Socha
//=====================================================

#include "PickUpItemBehavior.hpp"
#include "../Entities/NPC.hpp"
#include "../Pathfinding.hpp"
#include "../Map.hpp"

AIBehaviorRegistration PickUpItemBehavior::s_pickUpItemBehaviorRegistration(std::string("PickUpItem"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
PickUpItemBehavior::PickUpItemBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::OtherBehavior, behaviorNode),
m_targetItemLocation(-1, -1){
}

///=====================================================
/// 
///=====================================================
float PickUpItemBehavior::CalcUtility(){
	const Tile& tile = m_NPC->m_map->GetTileAtPosition(m_NPC->m_position);
	if (tile.m_items != nullptr){
		RECOVERABLE_ASSERT(!tile.m_items->m_backpack.empty());
		m_targetItemLocation = m_NPC->m_position;
		return 15.0f;
	}

	int closestDistanceSquared = INT_MAX;
	IntVec2 closestLocation;
	for (std::set<Tile*>::const_iterator visibleTileIter = m_NPC->m_visibleTiles.cbegin(); visibleTileIter != m_NPC->m_visibleTiles.cend(); ++visibleTileIter){
		Tile* visibleTile = *visibleTileIter;
		if (visibleTile->m_items != nullptr){
			int distanceSquared = CalcDistanceSquared(visibleTile->m_position, m_NPC->m_position);
			if (distanceSquared < closestDistanceSquared){
				closestDistanceSquared = distanceSquared;
				closestLocation = visibleTile->m_position;
			}
		}
	}

	if (closestDistanceSquared != INT_MAX){
		m_targetItemLocation = closestLocation;
		float utility = 9.0f - sqrt((float)closestDistanceSquared);
		if (utility < 0.0f)
			return 0.0f;
		return utility;
	}

	m_targetItemLocation = IntVec2(-1, -1);
	return 0.0f;
}

///=====================================================
/// 
///=====================================================
bool PickUpItemBehavior::Think(){
	RECOVERABLE_ASSERT(m_targetItemLocation.x != -1);

	const Tile& tile = m_NPC->m_map->GetTileAtPosition(m_NPC->m_position);
	if (tile.m_items != nullptr){
		RECOVERABLE_ASSERT(!tile.m_items->m_backpack.empty());
		m_NPC->PickUpItem();

		m_targetItemLocation = IntVec2(-1, -1);
		return true;
	}


	Path pathToTarget;
	pathToTarget.CalcPath(true, m_NPC, m_NPC->m_position, m_targetItemLocation, m_NPC->m_map, false, true);
	if (pathToTarget.m_path.size() < 2){
		return false;
	}
	else if (!m_NPC->CanMoveToLocation(pathToTarget.m_path[1]->m_position)){
		pathToTarget.Reset();
		pathToTarget.CalcPath(true, m_NPC, m_NPC->m_position, m_targetItemLocation, m_NPC->m_map, true, true);
		if (pathToTarget.m_path.size() < 2)
			return false;
		if (CalcDistanceSquared(m_NPC->m_position, m_targetItemLocation) < CalcDistanceSquared(pathToTarget.m_path[1]->m_position, m_targetItemLocation))
			return false;

		if (m_NPC->CanMoveToLocation(pathToTarget.m_path[1]->m_position)){
			m_NPC->MoveToLocation(pathToTarget.m_path[1]->m_position);
			return true;
		}
	}
	else{
		m_NPC->MoveToLocation(pathToTarget.m_path[1]->m_position);
		return true;
	}

	return false;
}
