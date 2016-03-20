//=====================================================
// RetreatBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_RetreatBehavior__
#define __included_RetreatBehavior__

#include "BaseAIBehavior.hpp"

class RetreatBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_chaseBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new RetreatBehavior(name, behaviorNode); }

public:
	RetreatBehavior(const std::string& name, const XMLNode& behaviorNode);

	int m_retreatCount;

	float CalcUtility();
	bool Think();
};

#endif