//=====================================================
// ChaseBehavior.cpp
// by Andrew Socha
//=====================================================

#include "ChaseBehavior.hpp"
#include "../Pathfinding.hpp"
#include "../Entities/NPC.hpp"
#include "../Entities/Feature.hpp"
#include "BaseAIBehavior.hpp"
#include "../Entities/Entity.hpp"
#include "../Map.hpp"

AIBehaviorRegistration ChaseBehavior::s_chaseBehaviorRegistration(std::string("Chase"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
ChaseBehavior::ChaseBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::Attack, behaviorNode){
}

///=====================================================
/// 
///=====================================================
float ChaseBehavior::CalcUtility(){
	if (m_NPC->m_targetEnemy != nullptr){
		float distance = CalcDistance(m_NPC->m_position, m_NPC->m_targetEnemy->m_position);
		if (distance < 2.0f)
			return 0.0f;

		float chaseUtility = 13.0f - distance;  //higher desire to chase the closer the target is
		chaseUtility -= Entity::s_turnCount - m_NPC->m_targetEnemy->m_turn;

		if (chaseUtility > 0.0f){
			if (m_NPC->m_targetEnemy->m_isFromYellForHelp){
				chaseUtility += distance * 0.5f; //only account for half the distance so we chase from further away
				chaseUtility *= 2.0f;
			}

			return chaseUtility * m_NPC->m_bravery;
		}
	}
	return 0.0f;
}

///=====================================================
/// 
///=====================================================
bool ChaseBehavior::Think(){
	IntVec2 nextStep = CalcNextPathStep();
	if (nextStep.x != -1){
		if (nextStep.x == -2){
			m_NPC->m_ignoredBehaviors.push_back(this);
			m_NPC->PlanNextThink(false);
			return m_NPC->Think(false);
		}
		
		Feature* feature = m_NPC->m_map->GetTileAtPosition(nextStep).m_feature;
		if (feature != nullptr && feature->m_isUsed == false){
			feature->UseFeature(*m_NPC);
			return true;
		}

		m_NPC->MoveToLocation(nextStep);
		return true;
	}

	return false;
}

///=====================================================
/// 
///=====================================================
IntVec2 ChaseBehavior::CalcNextPathStep() const{
	FATAL_ASSERT(m_NPC->m_targetEnemy != nullptr);

	if (m_NPC->m_position == m_NPC->m_targetEnemy->m_position)
		return IntVec2(-1, -1);

	Path pathToTarget;
	pathToTarget.CalcPath(!m_NPC->m_targetEnemy->m_isFromYellForHelp, m_NPC, m_NPC->m_position, m_NPC->m_targetEnemy->m_position, m_NPC->m_map, false, true, !m_NPC->m_canUseFeatures);
	if (pathToTarget.m_path.size() >= 2){
		Feature* feature = m_NPC->m_map->GetTileAtPosition(pathToTarget.m_path[1]->m_position).m_feature;
		if (m_NPC->CanMoveToLocation(pathToTarget.m_path[1]->m_position) || (m_NPC->m_canUseFeatures && feature != nullptr && !feature->m_isUsed)){
			return pathToTarget.m_path[1]->m_position;
		}


		pathToTarget.Reset();
		pathToTarget.CalcPath(!m_NPC->m_targetEnemy->m_isFromYellForHelp, m_NPC, m_NPC->m_position, m_NPC->m_targetEnemy->m_position, m_NPC->m_map, true, true);

		if (pathToTarget.m_path.empty()) //no path
			return IntVec2(-2, -2);
		if (pathToTarget.m_path.size() < 2) //already next to target
			return IntVec2(-1, -1);
		if (CalcDistanceSquared(m_NPC->m_position, m_NPC->m_targetEnemy->m_position) < CalcDistanceSquared(pathToTarget.m_path[1]->m_position, m_NPC->m_targetEnemy->m_position))
			return IntVec2(-1, -1);

		if (m_NPC->CanMoveToLocation(pathToTarget.m_path[1]->m_position))
			return pathToTarget.m_path[1]->m_position;
	}

	if (pathToTarget.m_path.empty()) //no path
		return IntVec2(-2, -2);
		
	return IntVec2(-1, -1);
}
