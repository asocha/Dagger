//=====================================================
// RetreatToAllyBehavior.cpp
// by Andrew Socha
//=====================================================

#include "RetreatToAllyBehavior.hpp"
#include "../Pathfinding.hpp"
#include "../Entities/NPC.hpp"

AIBehaviorRegistration RetreatToAllyBehavior::s_retreatToAllyBehaviorRegistration(std::string("RetreatToAlly"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
RetreatToAllyBehavior::RetreatToAllyBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::Retreat, behaviorNode),
m_avoidEnemies(true){
}

///=====================================================
/// 
///=====================================================
float RetreatToAllyBehavior::CalcUtility(){
	return 0.0f; //only occurs if the AI system wants the monster to do this
}

///=====================================================
/// 
///=====================================================
bool RetreatToAllyBehavior::Think(){
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
IntVec2 RetreatToAllyBehavior::CalcNextPathStep() const{
	FATAL_ASSERT(m_NPC->m_targetAlly != nullptr);
	FATAL_ASSERT(m_avoidEnemies == false || m_NPC->m_targetEnemy != nullptr);
	if (m_NPC->m_position == m_NPC->m_targetAlly->m_position){
		return IntVec2(-1, -1);
	}

	Path pathToTarget;
	pathToTarget.CalcPath(!m_NPC->m_targetAlly->m_isFromYellForHelp, m_NPC, m_NPC->m_position, m_NPC->m_targetAlly->m_position, m_NPC->m_map, false, true);
	if (pathToTarget.m_path.size() < 2){
		return IntVec2(-1, -1);
	}
	else if (!m_NPC->CanMoveToLocation(pathToTarget.m_path[1]->m_position)
			|| (m_avoidEnemies && CalcDistanceSquared(m_NPC->m_position, m_NPC->m_targetEnemy->m_position) <= CalcDistanceSquared(pathToTarget.m_path[1]->m_position, m_NPC->m_targetEnemy->m_position))){
		pathToTarget.Reset();
		pathToTarget.CalcPath(!m_NPC->m_targetAlly->m_isFromYellForHelp, m_NPC, m_NPC->m_position, m_NPC->m_targetAlly->m_position, m_NPC->m_map, true, true);
		if (pathToTarget.m_path.size() < 2)
			return IntVec2(-1, -1);
		if (m_avoidEnemies && CalcDistanceSquared(m_NPC->m_position, m_NPC->m_targetEnemy->m_position) > CalcDistanceSquared(pathToTarget.m_path[1]->m_position, m_NPC->m_targetEnemy->m_position))
			return IntVec2(-1, -1);

		if (m_NPC->CanMoveToLocation(pathToTarget.m_path[1]->m_position))
			return pathToTarget.m_path[1]->m_position;
		else
			return IntVec2(-1, -1);
	}
	else{
		return pathToTarget.m_path[1]->m_position;
	}
}
