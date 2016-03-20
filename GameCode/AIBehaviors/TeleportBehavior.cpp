//=====================================================
// TeleportBehavior.cpp
// by Andrew Socha
//=====================================================

#include "TeleportBehavior.hpp"
#include "../Entities/NPC.hpp"
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../MessageBar.hpp"

AIBehaviorRegistration TeleportBehavior::s_teleportBehaviorRegistration(std::string("Teleport"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
TeleportBehavior::TeleportBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::Attack, behaviorNode),
m_healthThreshold(0.0f),
m_chance(0.0f),
m_phrase("casts Teleport"),
m_numUses(-1){
	m_healthThreshold = GetFloatAttribute(behaviorNode, "healthThreshold", m_healthThreshold);
	m_chance = GetFloatAttribute(behaviorNode, "chance", m_chance);
	m_phrase = GetStringAttribute(behaviorNode, "phrase", m_phrase);
	m_numUses = GetIntAttribute(behaviorNode, "numUses", m_numUses);
}

///=====================================================
/// 
///=====================================================
float TeleportBehavior::CalcUtility(){
	if (m_numUses == 0)
		return 0.0f;

	if ((m_NPC->m_health / m_NPC->m_maxHealth) > m_healthThreshold)
		return 0.0f;

	if (GetRandomFloatZeroToOne() > m_chance)
		return 0.0f;

	return 3.0f * (float)(m_NPC->m_maxHealth - m_NPC->m_health);
}

///=====================================================
/// 
///=====================================================
bool TeleportBehavior::Think(){
	if ((m_NPC->m_health / m_NPC->m_maxHealth) > m_healthThreshold){
		m_NPC->m_ignoredBehaviors.push_back(this);
		m_NPC->PlanNextThink(false);
		return m_NPC->Think(false);
	}

	std::vector<Tile*>& airTiles = m_NPC->m_map->GetAllTilesOfType(TileType::Air, true);
	if (airTiles.empty())
		return false;

	std::vector<Tile*>::const_iterator tileIter = airTiles.cbegin() + GetRandomIntLessThan(airTiles.size());
	Tile* tile = *tileIter;

	while (CalcDistanceSquared(tile->m_position, m_NPC->m_position) <= (5 * 5)){
		airTiles.erase(tileIter);

		if (airTiles.empty())
			return false;

		tileIter = airTiles.cbegin() + GetRandomIntLessThan(airTiles.size());
		tile = *tileIter;
	}

	m_NPC->MoveToLocation(tile->m_position);
	m_NPC->TakeDamage(-(int)((float)m_NPC->m_maxHealth * m_healthThreshold), m_NPC);

	if (m_numUses > 0)
		--m_numUses;

	if (m_NPC->CanSeePlayer())
		s_theMessageBar->m_messages.push_back(m_NPC->m_name + " " + m_phrase + " and disappears!");

	return true;
}
