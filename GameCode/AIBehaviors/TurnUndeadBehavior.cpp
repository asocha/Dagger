//=====================================================
// TurnUndeadBehavior.cpp
// by Andrew Socha
//=====================================================

#include "TurnUndeadBehavior.hpp"
#include "../Entities/NPC.hpp"
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../MessageBar.hpp"
#include "Engine/Core/StringTable.hpp"
#include "../Faction.hpp"
#include "../CombatSystem.hpp"

AIBehaviorRegistration TurnUndeadBehavior::s_turnUndeadBehaviorRegistration(std::string("TurnUndead"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
TurnUndeadBehavior::TurnUndeadBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::Attack, behaviorNode),
m_factionName("Undead"),
m_radius(0),
m_damageRange(0, 0){
	m_damageRange = IntVec2(GetIntAttribute(behaviorNode, "minDamage", m_damageRange.x), GetIntAttribute(behaviorNode, "maxDamage", m_damageRange.y));
	m_factionName = GetStringAttribute(behaviorNode, "factionName", m_factionName);
	m_radius = GetIntAttribute(behaviorNode, "radius", m_radius);
}

///=====================================================
/// 
///=====================================================
float TurnUndeadBehavior::CalcUtility(){
	int numTargetActors = 0;
	for (std::vector<Actor*>::const_iterator actorIter = m_NPC->m_visibleActors.cbegin(); actorIter != m_NPC->m_visibleActors.cend(); ++actorIter){
		Actor* actor = *actorIter;
		if (actor->m_faction != nullptr && (GetStringID(m_factionName) == actor->m_faction->m_factionID) && CalcManhattanDistance(m_NPC->m_position, actor->m_position) <= m_radius){
			++numTargetActors;
		}
	}

	if (numTargetActors >= 3){
		return 10.0f * (float)numTargetActors;
	}

	return 0.0f;
}

///=====================================================
/// 
///=====================================================
bool TurnUndeadBehavior::Think(){
	s_theMessageBar->m_messages.push_back("The " + m_NPC->m_name + " glows and cries vengeance against the " + m_factionName + "!");

	for (std::vector<Actor*>::const_iterator actorIter = m_NPC->m_visibleActors.cbegin(); actorIter != m_NPC->m_visibleActors.cend(); ++actorIter){
		Actor* actor = *actorIter;
		if (actor->m_faction != nullptr && (GetStringID(m_factionName) == actor->m_faction->m_factionID) && CalcManhattanDistance(m_NPC->m_position, actor->m_position) <= m_radius){
			AttackData attackData;
			attackData.m_attacker = m_NPC;
			attackData.m_target = RememberedActor::GetActorFromRememberedActor(*actor);
			attackData.m_damageRange = m_damageRange;

			attackData.m_attackName = "smites";
			attackData.m_hitChance = m_NPC->m_chanceToHit;

			CombatSystem::PerformAttack(attackData, true, true);
		}
	}

	return true;
}
