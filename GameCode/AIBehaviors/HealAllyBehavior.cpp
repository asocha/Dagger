//=====================================================
// HealAllyBehavior.cpp
// by Andrew Socha
//=====================================================

#include "HealAllyBehavior.hpp"
#include "../CombatSystem.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../Entities/NPC.hpp"
#include "../Faction.hpp"

AIBehaviorRegistration HealAllyBehavior::s_healAllyBehaviorRegistration(std::string("HealAlly"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
HealAllyBehavior::HealAllyBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::Aide, behaviorNode),
m_healRange(0, 0),
m_range(0){
	m_healRange = IntVec2(GetIntAttribute(behaviorNode, "minHealing", m_healRange.x), GetIntAttribute(behaviorNode, "maxHealing", m_healRange.y));
	m_range = GetIntAttribute(behaviorNode, "range", m_range);
}

///=====================================================
/// 
///=====================================================
float HealAllyBehavior::CalcUtility(){
	m_healTarget = nullptr;
	int lowestAllyHealth = INT_MAX;

	for (std::vector<Actor*>::const_iterator actorIter = m_NPC->m_visibleActors.cbegin(); actorIter != m_NPC->m_visibleActors.cend(); ++actorIter){
		Actor* actor = *actorIter;
		if (actor == m_NPC || actor->IsPlayer())
			continue;

		NPC* npc = (NPC*)actor;

		if (npc->m_health != npc->m_maxHealth && npc->m_health < lowestAllyHealth && npc->m_faction != nullptr && m_NPC->GetFactionStatus(npc->m_faction->m_factionID, npc->m_ID) > 0 && CalcDistanceSquared(m_NPC->m_position, npc->m_position) <= (m_range * m_range)){
			lowestAllyHealth = npc->m_health;
			m_healTarget = npc;
		}
	}


	if (m_healTarget != nullptr){
		float utility = 13.0f + (float)(m_healTarget->m_maxHealth - m_healTarget->m_health);
		if (m_healTarget->m_isInvulnerable)
			utility *= 0.5f;

		return utility;
	}
	return 0.0f;
}

///=====================================================
/// 
///=====================================================
bool HealAllyBehavior::Think(){
	FATAL_ASSERT(m_healTarget != nullptr);

	if (m_healTarget->m_isDead || m_healTarget->m_health == m_healTarget->m_maxHealth || CalcDistanceSquared(m_NPC->m_position, m_healTarget->m_position) > (m_range * m_range)){
		m_NPC->m_ignoredBehaviors.push_back(this);
		m_NPC->PlanNextThink(false);
		return m_NPC->Think(false);
	}

	//heal by doing negative damage
	AttackData meleeAttackData;
	meleeAttackData.m_target = m_healTarget;
	meleeAttackData.m_attacker = m_NPC;
	meleeAttackData.m_damageRange = -m_healRange;
	meleeAttackData.m_attackName = "healed";
	meleeAttackData.m_hitChance = 1.0f;

	CombatSystem::PerformAttack(meleeAttackData, true, true);

	//tell target not to sacrifice himself this turn
	if (!m_healTarget->IsPlayer()){
		NPC* healTargetNPC = (NPC*)m_healTarget;
		BaseAIBehavior* invulBehavior = healTargetNPC->FindBehaviorByName("InvulnerabilityBuff");
		if (invulBehavior != nullptr && invulBehavior == healTargetNPC->m_plannedBehavior){
			healTargetNPC->m_ignoredBehaviors.push_back(invulBehavior);
			healTargetNPC->PlanNextThink(false);
		}
	}

	return true;
}
