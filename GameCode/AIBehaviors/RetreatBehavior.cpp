//=====================================================
// RetreatBehavior.cpp
// by Andrew Socha
//=====================================================

#include "RetreatBehavior.hpp"
#include "../Pathfinding.hpp"
#include "../Entities/NPC.hpp"
#include "../MessageBar.hpp"

AIBehaviorRegistration RetreatBehavior::s_chaseBehaviorRegistration(std::string("Retreat"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
RetreatBehavior::RetreatBehavior(const std::string& name, const XMLNode& behaviorNode) :
BaseAIBehavior(name, BehaviorType::Retreat, behaviorNode),
m_retreatCount(0){
}

///=====================================================
/// 
///=====================================================
float RetreatBehavior::CalcUtility(){
	return 0.0f; //can only be used if the AI System wants the monster to use it
}

///=====================================================
/// 
///=====================================================
bool RetreatBehavior::Think(){
	FATAL_ASSERT(m_NPC->m_targetEnemy != nullptr);

	++m_retreatCount;

	if (m_retreatCount > 5){
		if (GetRandomIntInRange(5, 14) < m_retreatCount){
			//stop retreating after we have done so for a little bit
			m_NPC->m_ignoredBehaviors.push_back(this);
			m_NPC->PlanNextThink(false);
			bool didThink = m_NPC->Think(false);
			m_NPC->m_ignoreRetreatStrategy = true;

			if (m_NPC->CanSeePlayer())
				s_theMessageBar->m_messages.push_back(m_NPC->m_name + " turns to fight!");

			if (m_NPC->m_bravery < 1.5f)
				m_NPC->YellForHelp(RememberedActor::GetActorFromRememberedActor(*m_NPC->m_targetEnemy));

			return didThink;
		}
	}

	IntVec2 retreatPosition = m_NPC->m_position;
	if (m_NPC->m_targetEnemy->m_position.x < m_NPC->m_position.x)
		++retreatPosition.x;
	else if (m_NPC->m_targetEnemy->m_position.x > m_NPC->m_position.x)
		--retreatPosition.x;
	if (m_NPC->m_targetEnemy->m_position.y < m_NPC->m_position.y)
		++retreatPosition.y;
	else if (m_NPC->m_targetEnemy->m_position.y > m_NPC->m_position.y)
		--retreatPosition.y;

	if (retreatPosition.x == m_NPC->m_position.x){ //if running horizontally, try to move diagonally
		int horizontalChange = (GetRandomIntInRange(0, 1) == 1) ? -1 : 1;
		IntVec2 newRetreatPosition(retreatPosition.x + horizontalChange, retreatPosition.y);
		if (m_NPC->CanMoveToLocation(newRetreatPosition)){
			m_NPC->MoveToLocation(newRetreatPosition);
			return true;
		}

		newRetreatPosition.x = retreatPosition.x - horizontalChange;
		if (m_NPC->CanMoveToLocation(newRetreatPosition)){
			m_NPC->MoveToLocation(newRetreatPosition);
			return true;
		}
	}
	else if (retreatPosition.y == m_NPC->m_position.y){ //if running vertically, try to move diagonally
		int verticalChange = (GetRandomIntInRange(0, 1) == 1) ? -1 : 1;
		IntVec2 newRetreatPosition(retreatPosition.x, retreatPosition.y + verticalChange);
		if (m_NPC->CanMoveToLocation(newRetreatPosition)){
			m_NPC->MoveToLocation(newRetreatPosition);
			return true;
		}

		newRetreatPosition.y = retreatPosition.y - verticalChange;
		if (m_NPC->CanMoveToLocation(newRetreatPosition)){
			m_NPC->MoveToLocation(newRetreatPosition);
			return true;
		}
	}
	
	if (m_NPC->CanMoveToLocation(retreatPosition)){ //move directly away from our target enemy
		m_NPC->MoveToLocation(retreatPosition);
		return true;
	}
	else if (retreatPosition.x != m_NPC->m_position.x && retreatPosition.y != m_NPC->m_position.y){ //if we couldn't move diagonally away, try horizontally away
		IntVec2 newRetreatPosition(m_NPC->m_position.x, retreatPosition.y);
		if (m_NPC->CanMoveToLocation(newRetreatPosition)){
			m_NPC->MoveToLocation(newRetreatPosition);
			return true;
		}

		newRetreatPosition = IntVec2(retreatPosition.x, m_NPC->m_position.y);
		if (m_NPC->CanMoveToLocation(newRetreatPosition)){
			m_NPC->MoveToLocation(newRetreatPosition);
			return true;
		}
	}

	//we are unable to retreat, so instead we will never retreat again so we don't just wait in a corner awkwardly.
	m_NPC->m_ignoredBehaviors.push_back(this);
	m_NPC->PlanNextThink(false);
	bool didThink = m_NPC->Think(false);
	m_NPC->m_ignoreRetreatStrategy = true;

	if (m_retreatCount > 2){
		if (m_NPC->CanSeePlayer())
			s_theMessageBar->m_messages.push_back(m_NPC->m_name + " feels trapped and turns to fight!");

		if (m_NPC->m_bravery < 1.5f)
			m_NPC->YellForHelp(RememberedActor::GetActorFromRememberedActor(*m_NPC->m_targetEnemy));
	}

	return didThink;
}
