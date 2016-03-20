//=====================================================
// CombatSystem.cpp
// by Andrew Socha
//=====================================================

#include "CombatSystem.hpp"
#include "Entities/Actor.hpp"
#include "MessageBar.hpp"
#include "Faction.hpp"

///=====================================================
/// assumes weapons damage bonus is accounted for before being called
///=====================================================
void CombatSystem::PerformAttack(AttackData& attack, bool ignoreArmor /*= false*/, bool ignoreWeapons /*= false*/){
	FATAL_ASSERT(attack.m_attacker != nullptr);
	FATAL_ASSERT(attack.m_target != nullptr);
	RECOVERABLE_ASSERT(!attack.m_attacker->m_isDead);
	RECOVERABLE_ASSERT(!attack.m_target->m_isDead);

	std::string targetNameToLower = attack.m_target->m_name;
	std::transform(attack.m_target->m_name.begin(), attack.m_target->m_name.end(), targetNameToLower.begin(), ::tolower);

	if (targetNameToLower != "you")
		targetNameToLower = "a " + targetNameToLower;

	if (attack.m_target->IsActor()){
		Actor* targetActor = (Actor*)attack.m_target;
		if (targetActor->m_isInvulnerable){
			attack.m_didHit = false;
			goto skipDamage;
		}
	}

	if (attack.m_hitChance < GetRandomFloatZeroToOne()){
		attack.m_didHit = false;
		goto skipDamage;
	}

	attack.m_didHit = true;

	if (attack.m_attacker->m_inventory != nullptr && !ignoreWeapons){
		std::map<Item::EquipmentSlot, Item*>::const_iterator equipmentIter = attack.m_attacker->m_inventory->m_equipment.find(Item::WeaponSlot);
		if (equipmentIter != attack.m_attacker->m_inventory->m_equipment.cend())
			attack.m_damageRange += equipmentIter->second->m_damage;
	}

	attack.m_actualDamageDone = GetRandomIntInRange(std::min(attack.m_damageRange.x, attack.m_damageRange.y), std::max(attack.m_damageRange.x, attack.m_damageRange.y));

	if (attack.m_target->IsActor()){
		Actor* targetActor = (Actor*)attack.m_target;

		//reduce damage with armor
		if (targetActor->m_inventory != nullptr && !ignoreArmor){
			float damageReductionPercent = 0.0f;
			for (std::map<Item::EquipmentSlot, Item*>::const_iterator equipmentIter = targetActor->m_inventory->m_equipment.cbegin(); equipmentIter != targetActor->m_inventory->m_equipment.cend(); ++equipmentIter){
				damageReductionPercent += equipmentIter->second->m_damageReductionPercent;
			}

			attack.m_actualDamageDone = (int)((float)attack.m_actualDamageDone * (1.0f - damageReductionPercent));

			if (attack.m_actualDamageDone == 0){
				attack.m_didHit = false;
				goto skipDamage;
			}
		}
		
		//adjust faction/entity relationship status
		targetActor->AdjustFactionStatus(attack.m_attacker->m_faction->m_factionID, -attack.m_actualDamageDone, attack.m_attacker->m_ID);
	}

	attack.m_target->TakeDamage(attack.m_actualDamageDone, attack.m_attacker);

skipDamage:
	//determine if the player can see the combat, so we know whether or not to add a message about it
	bool playerCanSee = attack.m_attacker->IsPlayer() || attack.m_target->IsPlayer();
	if (!playerCanSee){
		playerCanSee = attack.m_attacker->CanSeePlayer();
	}
	if (!playerCanSee && attack.m_target->IsActor()){
		playerCanSee = ((Actor*)attack.m_target)->CanSeePlayer();
	}

	if (playerCanSee){
		if (attack.m_didHit){
			s_theMessageBar->m_messages.push_back(attack.m_attacker->m_name + " " + attack.m_attackName + " " + targetNameToLower + " for " + std::to_string(abs(attack.m_actualDamageDone)) + " damage.");
		}
		else if (attack.m_target->IsActor() && ((Actor*)attack.m_target)->m_isInvulnerable){
			s_theMessageBar->m_messages.push_back(attack.m_attacker->m_name + " attacked " + targetNameToLower + " but he was invulnerable.");
			attack.m_didKill = false;
			attack.m_actualDamageDone = 0;
			return;
		}
		else{
			s_theMessageBar->m_messages.push_back(attack.m_attacker->m_name + " missed " + targetNameToLower + ".");
			attack.m_didKill = false;
			attack.m_actualDamageDone = 0;
			return;
		}
	}

	//check if target died (always display this message)
	if (attack.m_target->m_isDead){
		attack.m_didKill = true;
		if (playerCanSee)
			s_theMessageBar->m_messages.push_back(attack.m_attacker->m_name + " killed " + targetNameToLower + "!");
	}
	else{
		attack.m_didKill = false;
	}
}
