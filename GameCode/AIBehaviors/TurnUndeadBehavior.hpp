//=====================================================
// TurnUndeadBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_TurnUndeadBehavior__
#define __included_TurnUndeadBehavior__

#include "BaseAIBehavior.hpp"
#include "Engine/Math/IntVec2.hpp"

class TurnUndeadBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_turnUndeadBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new TurnUndeadBehavior(name, behaviorNode); }

	std::string m_factionName;
	int m_radius;
	IntVec2 m_damageRange;

public:
	TurnUndeadBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();
};

#endif