//=====================================================
// WanderBehavior.cpp
// by Andrew Socha
//=====================================================

#include "WanderBehavior.hpp"
#include "../Entities/NPC.hpp"
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../Entities/Feature.hpp"

AIBehaviorRegistration WanderBehavior::s_wanderBehaviorRegistration(std::string("Wander"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
WanderBehavior::WanderBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::OtherBehavior, behaviorNode),
m_chanceToRest(0.0f),
m_healFromResting(0){
	m_chanceToRest = GetFloatAttribute(behaviorNode, "chanceToRest", m_chanceToRest);
	m_healFromResting = GetIntAttribute(behaviorNode, "healFromResting", m_healFromResting);
}

///=====================================================
/// 
///=====================================================
float WanderBehavior::CalcUtility(){
	return 1.0f;
}

///=====================================================
/// 
///=====================================================
bool WanderBehavior::Think(){
	if (m_NPC->m_health != m_NPC->m_maxHealth){
		if (GetRandomFloatZeroToOne() <= m_chanceToRest){
			m_NPC->TakeDamage(-m_healFromResting, m_NPC);
			return true;
		}
	}
	
	const std::vector<Tile*>& adjacentTiles = m_NPC->m_map->GetVisibleTilesWithinRadius(*m_NPC, m_NPC->m_position, TileType::Air, 1, true);
	if (!adjacentTiles.empty()){
		Tile* tile = adjacentTiles.at(GetRandomIntLessThan(adjacentTiles.size()));
		m_NPC->MoveToLocation(tile->m_position);

		return true;
	}
	else if (m_NPC->m_name == "Apprentice"){ //allow apprentices to open doors
		const std::vector<Tile*>& nearbyTiles = m_NPC->m_map->GetTilesWithinRadius(m_NPC->m_position);
		for (std::vector<Tile*>::const_iterator tileIter = nearbyTiles.cbegin(); tileIter != nearbyTiles.cend(); ++tileIter){
			Feature* feature = (*tileIter)->m_feature;
			if (feature != nullptr && !feature->m_isUsed){
				feature->UseFeature(*m_NPC);
				return true;
			}
		}
	}

	return false;
}
