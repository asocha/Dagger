//=====================================================
// DefendLocationBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_DefendLocationBehavior__
#define __included_DefendLocationBehavior__

#include "PickUpItemBehavior.hpp"

class DefendLocationBehavior : public PickUpItemBehavior{
private:
	static AIBehaviorRegistration s_defendLocationBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new DefendLocationBehavior(name, behaviorNode); }

	#define m_defenceLocation m_targetItemLocation
public:
	DefendLocationBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
};

#endif