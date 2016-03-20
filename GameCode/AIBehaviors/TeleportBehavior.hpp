//=====================================================
// TeleportBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_TeleportBehavior__
#define __included_TeleportBehavior__

#include "BaseAIBehavior.hpp"

class TeleportBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_teleportBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new TeleportBehavior(name, behaviorNode); }

	float m_healthThreshold;
	int m_numUses;
	std::string m_phrase;
	float m_chance;

public:
	TeleportBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();
};

#endif