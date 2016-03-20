//=====================================================
// MeleeAttackBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_MeleeAttackBehavior__
#define __included_MeleeAttackBehavior__

#include "BaseAIBehavior.hpp"
#include "Engine/Math/IntVec2.hpp"


class MeleeAttackBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_meleeAttackBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new MeleeAttackBehavior(name, behaviorNode); }

	static XMLNode s_behaviorNode;

	std::string m_attackName;

public:
	IntVec2 m_damageRange;

	MeleeAttackBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();

	inline const XMLNode& GetXMLNode() const { return s_behaviorNode; };
};

#endif