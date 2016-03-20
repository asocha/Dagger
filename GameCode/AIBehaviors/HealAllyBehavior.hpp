//=====================================================
// HealAllyBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_HealAllyBehavior__
#define __included_HealAllyBehavior__

#include "BaseAIBehavior.hpp"
#include "Engine/Math/IntVec2.hpp"
class Actor;


class HealAllyBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_healAllyBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new HealAllyBehavior(name, behaviorNode); }

	static XMLNode s_behaviorNode;

	IntVec2 m_healRange;
	int m_range;

	Actor* m_healTarget;

public:
	HealAllyBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();

	inline const XMLNode& GetXMLNode() const { return s_behaviorNode; };
};

#endif