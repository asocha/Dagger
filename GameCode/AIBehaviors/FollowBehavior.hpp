//=====================================================
// FollowBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_FollowBehavior__
#define __included_FollowBehavior__

#include "BaseAIBehavior.hpp"
#include "Engine/Math/IntVec2.hpp"
struct RememberedActor;

class FollowBehavior : public BaseAIBehavior{
private:
	static AIBehaviorRegistration s_followBehaviorRegistration;
	inline static BaseAIBehavior* CreateAIBehavior(const std::string& name, const XMLNode& behaviorNode){ return new FollowBehavior(name, behaviorNode); }

	std::string m_leaderName;
	float m_distanceThreshold;
	RememberedActor* m_followActor;

public:
	FollowBehavior(const std::string& name, const XMLNode& behaviorNode);

	float CalcUtility();
	bool Think();

	IntVec2 CalcNextPathStep() const;
};

#endif