//=====================================================
// AOEBuffBehavior.cpp
// by Andrew Socha
//=====================================================

#include "AOEBuffBehavior.hpp"
#include "../Entities/NPC.hpp"
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../MessageBar.hpp"
#include "Engine/Core/StringTable.hpp"
#include "../Faction.hpp"
#include "../CombatSystem.hpp"
#include "MeleeAttackBehavior.hpp"

AIBehaviorRegistration AOEBuffBehavior::s_AOEBuffBehaviorRegistration(std::string("AOEBuff"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
AOEBuffBehavior::AOEBuffBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::Aide, behaviorNode),
m_radius(0),
m_damageBonus(0, 0),
m_healthBonus(0),
m_hasBeenUsed(false){
	m_damageBonus = IntVec2(GetIntAttribute(behaviorNode, "minDamage", m_damageBonus.x), GetIntAttribute(behaviorNode, "maxDamage", m_damageBonus.y));
	m_healthBonus = GetIntAttribute(behaviorNode, "health", m_healthBonus);
	m_radius = GetIntAttribute(behaviorNode, "radius", m_radius);
}

///=====================================================
/// 
///=====================================================
float AOEBuffBehavior::CalcUtility(){
	if (m_hasBeenUsed)
		return 0.0f;

	int numTargetAllies = 1; //start at 1 as we target ourselves but do not see ourselves
	int numFarAwayAllies = 0;
	for (std::vector<Actor*>::const_iterator actorIter = m_NPC->m_visibleActors.cbegin(); actorIter != m_NPC->m_visibleActors.cend(); ++actorIter){
		Actor* actor = *actorIter;
		if (actor->m_faction != nullptr && (m_NPC->GetFactionStatus(actor->m_faction->m_factionID, actor->m_ID) > 0)){
			if (CalcDistanceSquared(m_NPC->m_position, actor->m_position) <= (m_radius * m_radius))
				++numTargetAllies;
			else
				++numFarAwayAllies; //if allies are visible but not in range, be less likely to use buff
		}
	}

	if (numTargetAllies >= 4 || (numTargetAllies == 3 && numFarAwayAllies == 0)){
		float utility = 3.0f * (float)numTargetAllies - 2.0f * (float)numFarAwayAllies;
		if (utility > 0.0f)
			return utility;
	}

	return 0.0f;
}

///=====================================================
/// 
///=====================================================
bool AOEBuffBehavior::Think(){
	FATAL_ASSERT(m_hasBeenUsed == false);
	s_theMessageBar->m_messages.push_back(m_NPC->m_name + " buffs his allies with great strength!");

	for (std::vector<Actor*>::const_iterator actorIter = m_NPC->m_visibleActors.cbegin(); actorIter != m_NPC->m_visibleActors.cend(); ++actorIter){
		Actor* actor = *actorIter;
		if (actor->m_faction != nullptr && (m_NPC->GetFactionStatus(actor->m_faction->m_factionID, actor->m_ID) > 0) && CalcDistanceSquared(m_NPC->m_position, actor->m_position) <= (m_radius * m_radius)){
			actor->m_health += m_healthBonus;
			actor->m_maxHealth += m_healthBonus;

			if (!actor->IsPlayer()){
				((NPC*)actor)->m_bravery += 0.5f;
				MeleeAttackBehavior* attackBehavior = (MeleeAttackBehavior*)((NPC*)actor)->FindBehaviorByName("MeleeAttack");
				if (attackBehavior != nullptr)
					attackBehavior->m_damageRange += m_damageBonus;
			}
			else{
				FATAL_ERROR(); //do something here if I ever want to buff the player
			}
		}
	}

	//buff the caster
// 	m_NPC->m_health += m_healthBonus;
// 	m_NPC->m_maxHealth += m_healthBonus;
// 	m_NPC->m_bravery += 0.5f;
// 	MeleeAttackBehavior* attackBehavior = (MeleeAttackBehavior*)m_NPC->FindBehaviorByName("MeleeAttack");
// 	if (attackBehavior != nullptr)
// 		attackBehavior->m_damageRange += m_damageBonus;

	m_hasBeenUsed = true;

	return true;
}
