//=====================================================
// SummonAllyBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_SummonAllyBehavior__
#define __included_SummonAllyBehavior__

#include "BaseAIBehavior.hpp"

class SummonAllyBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_summonAllyBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new SummonAllyBehavior(name, behaviorNode); }

	std::string m_summonedActorName;

public:
	SummonAllyBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();
};

#endif