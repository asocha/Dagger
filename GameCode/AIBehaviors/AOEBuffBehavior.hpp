//=====================================================
// AOEBuffBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_AOEBuffBehavior__
#define __included_AOEBuffBehavior__

#include "BaseAIBehavior.hpp"
#include "Engine/Math/IntVec2.hpp"

class AOEBuffBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_AOEBuffBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new AOEBuffBehavior(name, behaviorNode); }

	int m_radius;
	IntVec2 m_damageBonus;
	int m_healthBonus;

public:
	bool m_hasBeenUsed;

	AOEBuffBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();
};

#endif