//=====================================================
// PickUpItemBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_PickUpItemBehavior__
#define __included_PickUpItemBehavior__

#include "BaseAIBehavior.hpp"
#include "Engine/Math/IntVec2.hpp"

class PickUpItemBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_pickUpItemBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new PickUpItemBehavior(name, behaviorNode); }

public:
	IntVec2 m_targetItemLocation;

	PickUpItemBehavior(const std::string& name, const XMLNode& behaviorNode);
	inline virtual ~PickUpItemBehavior(){}

	virtual float CalcUtility();
	bool Think();
};

#endif