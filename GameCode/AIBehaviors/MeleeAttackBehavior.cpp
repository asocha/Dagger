//=====================================================
// MeleeAttackBehavior.cpp
// by Andrew Socha
//=====================================================

#include "MeleeAttackBehavior.hpp"
#include "../CombatSystem.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../Entities/NPC.hpp"

AIBehaviorRegistration MeleeAttackBehavior::s_meleeAttackBehaviorRegistration(std::string("MeleeAttack"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
MeleeAttackBehavior::MeleeAttackBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::Attack, behaviorNode),
m_damageRange(0, 0),
m_attackName(""){
	m_damageRange = IntVec2(GetIntAttribute(behaviorNode, "minDamage", 0), GetIntAttribute(behaviorNode, "maxDamage", 0));
	m_attackName = GetStringAttribute(behaviorNode, "attack", "hit");
}

///=====================================================
/// 
///=====================================================
float MeleeAttackBehavior::CalcUtility(){
	if (m_NPC->m_targetEnemy != nullptr){
		if (m_NPC->m_targetEnemy->m_turn != Entity::s_turnCount)
			return 0.0f;

		if (CalcDistanceSquared(m_NPC->m_position, m_NPC->m_targetEnemy->m_position) < (2 * 2)){
			IntVec2 actualDamageRange = m_damageRange;
			if (m_NPC->m_inventory != nullptr){
				std::map<Item::EquipmentSlot, Item*>::const_iterator equipmentIter = m_NPC->m_inventory->m_equipment.find(Item::WeaponSlot);
				if (equipmentIter != m_NPC->m_inventory->m_equipment.cend())
					actualDamageRange += equipmentIter->second->m_damage;
			}

			float averageDamage = ((float)actualDamageRange.x + (float)actualDamageRange.y) * 0.5f;

			float attackUtility = 13.0f + averageDamage;
			if (m_NPC->m_targetEnemy->m_isFromYellForHelp)
				attackUtility *= 2.0f;

			return attackUtility * m_NPC->m_bravery;
		}
	}
	return 0.0f;
}

///=====================================================
/// 
///=====================================================
bool MeleeAttackBehavior::Think(){
	FATAL_ASSERT(m_NPC->m_targetEnemy != nullptr);
	RECOVERABLE_ASSERT(m_NPC->m_targetEnemy->m_turn == Entity::s_turnCount);
	RECOVERABLE_ASSERT(m_NPC->GetFactionStatus(m_NPC->m_targetEnemy->m_factionID, m_NPC->m_targetEnemy->m_ID) < 0);

	AttackData meleeAttackData;
	meleeAttackData.m_target = RememberedActor::GetActorFromRememberedActor(*m_NPC->m_targetEnemy);
	if (meleeAttackData.m_target->m_isDead){
		m_NPC->m_ignoredBehaviors.push_back(this);
		m_NPC->PlanNextThink(false);
		return m_NPC->Think(false);
	}

	meleeAttackData.m_attacker = m_NPC;
	meleeAttackData.m_damageRange = m_damageRange;
	meleeAttackData.m_attackName = m_attackName;
	meleeAttackData.m_hitChance = m_NPC->m_chanceToHit;

	CombatSystem::PerformAttack(meleeAttackData);

	return true;
}
