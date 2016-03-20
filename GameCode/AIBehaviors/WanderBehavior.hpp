//=====================================================
// WanderBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_WanderBehavior__
#define __included_WanderBehavior__

#include "BaseAIBehavior.hpp"

class WanderBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_wanderBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new WanderBehavior(name, behaviorNode); }

	float m_chanceToRest;
	int m_healFromResting;

public:
	WanderBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();
};

#endif