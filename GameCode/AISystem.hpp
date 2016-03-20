//=====================================================
// AISystem.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_AISystem__
#define __included_AISystem__

class Player;
struct RememberedActor;
class NPC;

class AISystem{
private:
	void AttackStrategy(const Player& player) const;
	void RetreatStrategy(const Player& player) const;
	RememberedActor* FindAllyToRetreatTo(NPC& retreatingNPC, bool retreatToBuffers, bool findClosest) const;
	void DefendStrategy() const;

public:
	enum Strategy{
		Default,
		Attack,
		Retreat,
		Defend
	};

	Strategy m_currentStrategy;

	AISystem();
	
	void PlanNextMove(const Player& player);
};

#endif