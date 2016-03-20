//=====================================================
// ChaseBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_ChaseBehavior__
#define __included_ChaseBehavior__

#include "BaseAIBehavior.hpp"

struct IntVec2;

class ChaseBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_chaseBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new ChaseBehavior(name, behaviorNode); }

public:
	ChaseBehavior(const std::string& name, const XMLNode& behaviorNode);

	virtual float CalcUtility();
	bool Think();

	IntVec2 CalcNextPathStep() const;
};

#endif