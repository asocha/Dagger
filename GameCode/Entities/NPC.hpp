//=====================================================
// NPC.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_NPC__
#define __included_NPC__

#include "Actor.hpp"
class BaseAIBehavior;

class NPC : public Actor{
private:
	bool m_yelledForHelp;
	bool m_canYellForHelp;

public:
	std::vector<BaseAIBehavior*> m_behaviors;
	BaseAIBehavior* m_plannedBehavior;
	std::vector<BaseAIBehavior*> m_ignoredBehaviors;
	bool m_ignoreRetreatStrategy;
	float m_bravery;
	static int s_mostRecentYellForHelpDueToPlayerTurn;

	NPC(const XMLNode& npcNode, OpenGLRenderer* renderer);
	NPC(const NPC& other, OpenGLRenderer* renderer, const XMLNode& saveData = XMLNode::emptyNode());
	~NPC();
	
	bool Think(bool isMainThink);
	void PlanNextThink(bool recomputeFieldOfView = true);
	BaseAIBehavior* FindBehaviorByName(const std::string& name) const;

	void TakeDamage(int damage, const Actor* attacker);
	void YellForHelp(const Actor* attacker);

	void SaveGame(XMLNode& entitiesNode) const;
};

#endif