//=====================================================
// FollowBehavior.cpp
// by Andrew Socha
//=====================================================

#include "FollowBehavior.hpp"
#include "../Pathfinding.hpp"
#include "../Entities/NPC.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"

AIBehaviorRegistration FollowBehavior::s_followBehaviorRegistration(std::string("Follow"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
FollowBehavior::FollowBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::OtherBehavior, behaviorNode),
m_leaderName(""),
m_distanceThreshold(-1),
m_followActor(nullptr){
	m_leaderName = GetStringAttribute(behaviorNode, "leaderName", m_leaderName);
	m_distanceThreshold = GetFloatAttribute(behaviorNode, "distanceThreshold", m_distanceThreshold);
}

///=====================================================
/// 
///=====================================================
float FollowBehavior::CalcUtility(){
	m_followActor = nullptr;

	for (std::map<int, RememberedActor>::const_iterator actorIter = m_NPC->m_previouslyVisibleActors.cbegin(); actorIter != m_NPC->m_previouslyVisibleActors.cend(); ++actorIter){
		const RememberedActor& actor = actorIter->second;
		if (actor.m_name == m_leaderName){
			int distanceSquared = CalcDistanceSquared(actor.m_position, m_NPC->m_position);
			if ((float)distanceSquared <= (m_distanceThreshold * m_distanceThreshold))
				return 0.0f;
			else{
				m_followActor = (RememberedActor*)&actor;
				return 5.0f;
			}
		}
	}

	return 0.0f;
}

///=====================================================
/// 
///=====================================================
bool FollowBehavior::Think(){
	IntVec2 nextStep = CalcNextPathStep();
	if (nextStep.x != -1){
		m_NPC->MoveToLocation(nextStep);

		return true;
	}
	else{
		m_NPC->m_ignoredBehaviors.push_back(this);
		m_NPC->PlanNextThink(false);
		return m_NPC->Think(false);
	}
}

///=====================================================
/// 
///=====================================================
IntVec2 FollowBehavior::CalcNextPathStep() const{
	FATAL_ASSERT(m_followActor != nullptr);

	Path pathToTarget;
	pathToTarget.CalcPath(true, m_NPC, m_NPC->m_position, m_followActor->m_position, m_NPC->m_map, false, true);
	if (pathToTarget.m_path.size() < 2){
		return IntVec2(-1, -1);
	}
	else if (!m_NPC->CanMoveToLocation(pathToTarget.m_path[1]->m_position)){
		pathToTarget.Reset();
		pathToTarget.CalcPath(true, m_NPC, m_NPC->m_position, m_followActor->m_position, m_NPC->m_map, true, true);
		if (pathToTarget.m_path.size() < 2)
			return IntVec2(-1, -1);

		if (m_NPC->CanMoveToLocation(pathToTarget.m_path[1]->m_position))
			return pathToTarget.m_path[1]->m_position;
		
		return IntVec2(-1, -1);
	}
	else{
		return pathToTarget.m_path[1]->m_position;
	}
}
