//=====================================================
// SummonAllyBehavior.cpp
// by Andrew Socha
//=====================================================

#include "SummonAllyBehavior.hpp"
#include "../Entities/NPC.hpp"
#include "../MessageBar.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../Factories/NPCFactory.hpp"
#include "../Map.hpp"


AIBehaviorRegistration SummonAllyBehavior::s_summonAllyBehaviorRegistration(std::string("SummonAlly"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
SummonAllyBehavior::SummonAllyBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::OtherBehavior, behaviorNode),
m_summonedActorName(""){
	m_summonedActorName = GetStringAttribute(behaviorNode, "ally");
	RECOVERABLE_ASSERT(m_summonedActorName != "");
}

///=====================================================
/// 
///=====================================================
float SummonAllyBehavior::CalcUtility(){
	if (m_NPC->m_visibleActors.empty()){ //summon when we get lonely and can't see anyone
		return 50.0f;
	}
	return 0.0f;
}

///=====================================================
/// 
///=====================================================
bool SummonAllyBehavior::Think(){
	const std::vector<Tile*>& nearbyAirTiles = m_NPC->m_map->GetVisibleTilesWithinRadius(*m_NPC, m_NPC->m_position, TileType::Air, 2, true);
	if (nearbyAirTiles.empty()){
		m_NPC->m_ignoredBehaviors.push_back(this);
		m_NPC->PlanNextThink(false);
		return m_NPC->Think(false);
	}	

	NPCFactory* npcFactory = NPCFactory::FindFactoryByName(m_summonedActorName);
	FATAL_ASSERT(npcFactory != nullptr);

	NPC* newNPC = npcFactory->SpawnNPC();
	
	Tile* randomAirTile = nearbyAirTiles.at(GetRandomIntLessThan(nearbyAirTiles.size()));
	newNPC->AddToMap(m_NPC->m_map, randomAirTile->m_position);

	s_theMessageBar->m_messages.push_back(m_NPC->m_name + " summoned a " + m_summonedActorName + "!");

	return true;
}
