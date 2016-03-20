//=====================================================
// CombatSystem.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_CombatSystem__
#define __included_CombatSystem__

class Actor;
class Entity;
#include <string>
#include "Engine/Math/IntVec2.hpp"

struct AttackData{
	Actor* m_attacker;
	Entity* m_target;
	std::string m_attackName;
	IntVec2 m_damageRange;
	float m_hitChance;

	bool m_didHit;
	bool m_didKill;
	int m_actualDamageDone;
};

class CombatSystem{
public:
	inline CombatSystem(){}

	static void PerformAttack(AttackData& attack, bool ignoreArmor = false, bool ignoreWeapons = false);
	
};

#endif