//=====================================================
// RetreatToAllyBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_RetreatToAllyBehavior__
#define __included_RetreatToAllyBehavior__

#include "BaseAIBehavior.hpp"
#include "Engine/Math/IntVec2.hpp"

class RetreatToAllyBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_retreatToAllyBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new RetreatToAllyBehavior(name, behaviorNode); }

public:
	bool m_avoidEnemies;
	RetreatToAllyBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();

	IntVec2 CalcNextPathStep() const;
};

#endif