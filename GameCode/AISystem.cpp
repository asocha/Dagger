//=====================================================
// AISystem.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "AISystem.hpp"
#include "Entities/Actor.hpp"
#include "Entities/NPC.hpp"
#include "AIBehaviors/RetreatToAllyBehavior.hpp"
#include "AIBehaviors/DefendLocationBehavior.hpp"
#include "Entities/Player.hpp"
#include "Faction.hpp"
#include "AIBehaviors/AOEBuffBehavior.hpp"
#include "AIBehaviors/RetreatBehavior.hpp"

///=====================================================
/// 
///=====================================================
AISystem::AISystem():
m_currentStrategy(Default){
}

///=====================================================
/// 
///=====================================================
void AISystem::PlanNextMove(const Player& player){
	int numChases = 0;
	int numAttacks = 0;
	int numTargetingThePlayer = 0;
	float attackerBravery = 0.0f;
	bool anNPCCanNotRetreat = false;

	for (Actors::const_iterator actorIter = Actor::s_actorsOnMap.cbegin(); actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
		Actor* actor = actorIter->second;

		if (actor->m_isDead || actor->GetFactionStatus(player.m_faction->m_factionID, player.m_ID) > 0)
			continue;

		if (actor->IsPlayer()){
			break;
		}

		NPC* npc = (NPC*)actor;
		if (npc->m_targetEnemy != nullptr && npc->m_targetEnemy->m_isPlayer && npc->m_targetEnemy->m_turn == Entity::s_turnCount){
			++numTargetingThePlayer;

			if (npc->m_ignoreRetreatStrategy == true)
				anNPCCanNotRetreat = true;

			if (npc->m_plannedBehavior->m_name == "Chase"){
				++numChases;
				if (npc->m_targetEnemy->m_isFromYellForHelp)
					attackerBravery += npc->m_bravery * 0.5f; //half bravery from NPCs that weren't planning on being aggressive
				else
					attackerBravery += npc->m_bravery;
			}
			else if (npc->m_plannedBehavior->m_name == "MeleeAttack"){
				++numAttacks;
				attackerBravery += npc->m_bravery;
			}
			else if (!npc->m_targetEnemy->m_isFromYellForHelp){
				attackerBravery += npc->m_bravery * 0.5f; //half bravery from NPCs that weren't planning on being aggressive
			}
		}
	}

	switch (m_currentStrategy)
	{
	case AISystem::Default:
		if (numAttacks != 0 || attackerBravery > 2.0f || anNPCCanNotRetreat || NPC::s_mostRecentYellForHelpDueToPlayerTurn != -1){
			m_currentStrategy = Attack;
			AttackStrategy(player);
			break;
		}
		else{
			m_currentStrategy = Retreat;
			RetreatStrategy(player);
			break;
		}
		break;
	case AISystem::Attack:
		if (numAttacks == 0 && attackerBravery <= 1.0f && !anNPCCanNotRetreat && (NPC::s_mostRecentYellForHelpDueToPlayerTurn + 10 < Entity::s_turnCount)){
			//reset any Retreat Strategy Ignoring from previous set of retreats
			for (Actors::const_iterator actorIter = Actor::s_actorsOnMap.cbegin(); actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
				Actor* actor = actorIter->second;
				if (!actor->IsPlayer()){
					NPC* npc = (NPC*)actor;
					npc->m_ignoreRetreatStrategy = false;

					RetreatBehavior* retreatBehavior = (RetreatBehavior*)npc->FindBehaviorByName("Retreat");
					if (retreatBehavior != nullptr){
						retreatBehavior->m_retreatCount = 0;
					}
				}
			}

			m_currentStrategy = Retreat;
			RetreatStrategy(player);
			break;
		}
		AttackStrategy(player);
		break;
	case AISystem::Retreat:
		if (numAttacks != 0 || attackerBravery > 2.0f || anNPCCanNotRetreat || (NPC::s_mostRecentYellForHelpDueToPlayerTurn + 10 >= Entity::s_turnCount)){
			m_currentStrategy = Attack;
			AttackStrategy(player);
			break;
		}
		RetreatStrategy(player);
		break;
	case AISystem::Defend:
		DefendStrategy();
		break;
	default:
		RECOVERABLE_ERROR();
		break;
	}
	
}

///=====================================================
/// 
///=====================================================
void AISystem::AttackStrategy(const Player& player) const{
	for (Actors::const_iterator actorIter = Actor::s_actorsOnMap.cbegin(); actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
		Actor* actor = actorIter->second;

		if (actor->m_isDead || actor->GetFactionStatus(player.m_faction->m_factionID, player.m_ID) > 0)
			continue;

		if (actor->IsPlayer()){
			break;
		}

		NPC* npc = (NPC*)actor;

		//if we are a healer, we are next to an enemy, and there are nearby allies, try to run away
		if (npc->FindBehaviorByName("HealAlly") != nullptr && (npc->m_plannedBehavior->m_behaviorType == BehaviorType::Attack)){
			if (npc->m_targetAlly != nullptr && npc->m_targetAlly->m_turn == Entity::s_turnCount && npc->m_targetEnemy->m_turn == Entity::s_turnCount && CalcDistanceSquared(npc->m_position, npc->m_targetEnemy->m_position) <= 8){
				BaseAIBehavior* retreatToAllyBehavior = npc->FindBehaviorByName("RetreatToAlly");
				if (retreatToAllyBehavior != nullptr){
					RememberedActor* allyToRetreatTo = FindAllyToRetreatTo(*npc, false, true);
					if (allyToRetreatTo != nullptr){
						//smartly retreat toward an ally if we can
						delete npc->m_targetAlly;
						npc->m_targetAlly = allyToRetreatTo;

						npc->m_plannedBehavior = retreatToAllyBehavior;
						continue;
					}
				}

				RetreatBehavior* retreatBehavior = (RetreatBehavior*)npc->FindBehaviorByName("Retreat");
				if (retreatBehavior != nullptr){
					retreatBehavior->m_retreatCount = 0;
					npc->m_plannedBehavior = retreatBehavior; //otherwise, dumbly retreat
					continue;
				}

				continue;
			}
		}

		//if a lot of monsters want to chase/attack, we should too as long as we know where the player is
		if (npc->m_targetEnemy != nullptr && npc->m_targetEnemy->m_isPlayer){
			if (npc->m_plannedBehavior->m_behaviorType != BehaviorType::Attack && npc->m_plannedBehavior->m_behaviorType != BehaviorType::Aide){ //if we're far away and a bunch of monsters want to chase, get a bit closer
				BaseAIBehavior* chaseBehavior = npc->FindBehaviorByName("Chase");
				RECOVERABLE_ASSERT(chaseBehavior != nullptr);
				if (chaseBehavior != nullptr){
					npc->m_plannedBehavior = chaseBehavior;
					continue;
				}
			}
		}

		//try to group up for buffing
		if (npc->m_plannedBehavior->m_name == "AOEBuff" || (npc->m_plannedBehavior->m_behaviorType != BehaviorType::Attack && npc->m_plannedBehavior->m_behaviorType != BehaviorType::Aide && npc->m_plannedBehavior->m_name != "PickUpItem")){
			BaseAIBehavior* retreatToAllyBehavior = npc->FindBehaviorByName("RetreatToAlly");
			if (retreatToAllyBehavior != nullptr){
				AOEBuffBehavior* buffBehavior = (AOEBuffBehavior*)npc->FindBehaviorByName("AOEBuff");
				if (buffBehavior == nullptr){
					RememberedActor* allyToRetreatTo = FindAllyToRetreatTo(*npc, true, true); //try to move to a buff-caster
					if (allyToRetreatTo != nullptr){
						delete npc->m_targetAlly;
						npc->m_targetAlly = allyToRetreatTo;

						npc->m_plannedBehavior = retreatToAllyBehavior;
						continue;
					}
				}
				else if (buffBehavior->m_hasBeenUsed == false && npc->m_plannedBehavior->m_name != "AOEBuff"){
					RememberedActor* allyToRetreatTo = FindAllyToRetreatTo(*npc, false, false); //try to move to someone who we can buff
					if (allyToRetreatTo != nullptr){
						delete npc->m_targetAlly;
						npc->m_targetAlly = allyToRetreatTo;

						npc->m_plannedBehavior = retreatToAllyBehavior;
						continue;
					}
				}
			}
		}
	}
}


///=====================================================
/// 
///=====================================================
void AISystem::RetreatStrategy(const Player& player) const{
	for (Actors::const_iterator actorIter = Actor::s_actorsOnMap.cbegin(); actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
		Actor* actor = actorIter->second;

		if (actor->m_isDead || actor->GetFactionStatus(player.m_faction->m_factionID, player.m_ID) > 0)
			continue;

		if (actor->IsPlayer()){
			break;
		}

		NPC* npc = (NPC*)actor;


		//try to group up for buffing
		if (npc->m_plannedBehavior->m_name == "AOEBuff" || (npc->m_plannedBehavior->m_behaviorType != BehaviorType::Attack && npc->m_plannedBehavior->m_behaviorType != BehaviorType::Aide && npc->m_plannedBehavior->m_name != "PickUpItem")){
			BaseAIBehavior* retreatToAllyBehavior = npc->FindBehaviorByName("RetreatToAlly");
			if (retreatToAllyBehavior != nullptr){
				AOEBuffBehavior* buffBehavior = (AOEBuffBehavior*)npc->FindBehaviorByName("AOEBuff");
				if (buffBehavior == nullptr){
					RememberedActor* allyToRetreatTo = FindAllyToRetreatTo(*npc, true, true); //try to move to a buff-caster
					if (allyToRetreatTo != nullptr){
						delete npc->m_targetAlly;
						npc->m_targetAlly = allyToRetreatTo;

						npc->m_plannedBehavior = retreatToAllyBehavior;
						continue;
					}
				}
				else if (buffBehavior->m_hasBeenUsed == false && npc->m_plannedBehavior->m_name != "AOEBuff"){
					RememberedActor* allyToRetreatTo = FindAllyToRetreatTo(*npc, false, false); //try to move to someone who we can buff
					if (allyToRetreatTo != nullptr){
						delete npc->m_targetAlly;
						npc->m_targetAlly = allyToRetreatTo;

						npc->m_plannedBehavior = retreatToAllyBehavior;
						continue;
					}
				}
			}
		}


		//don't chase if not many monsters want to chase/attack... retreat instead
		if (npc->m_targetEnemy != nullptr && npc->m_targetEnemy->m_isPlayer && npc->m_bravery < 2.0f && !npc->m_ignoreRetreatStrategy){
			if (npc->m_plannedBehavior->m_name == "Chase"){
				BaseAIBehavior* retreatToAllyBehavior = npc->FindBehaviorByName("RetreatToAlly");
				if (retreatToAllyBehavior != nullptr){
					RememberedActor* allyToRetreatTo = FindAllyToRetreatTo(*npc, false, true);
					if (allyToRetreatTo != nullptr){
						//smartly retreat toward an ally if we can
						delete npc->m_targetAlly;
						npc->m_targetAlly = allyToRetreatTo;

						npc->m_plannedBehavior = retreatToAllyBehavior;
						continue;
					}
				}

				BaseAIBehavior* retreatBehavior = npc->FindBehaviorByName("Retreat");
				if (retreatBehavior != nullptr){
					npc->m_plannedBehavior = retreatBehavior; //otherwise, dumbly retreat
					continue;
				}

				//if we don't know how to retreat, just choose a non-chase behavior
				npc->m_ignoredBehaviors.push_back(npc->m_plannedBehavior);
				npc->PlanNextThink(false);
			}
		}
	}
}

///=====================================================
/// //assumes NPC has RetreatToAllyBehavior
///=====================================================
RememberedActor* AISystem::FindAllyToRetreatTo(NPC& retreatingNPC, bool retreatToBuffers, bool findClosest) const{
	RetreatToAllyBehavior* retreatToAllyBehavior = (RetreatToAllyBehavior*)retreatingNPC.FindBehaviorByName("RetreatToAlly");
	retreatToAllyBehavior->m_avoidEnemies = !retreatToBuffers && findClosest;

	if (retreatToAllyBehavior == nullptr){
		RECOVERABLE_ERROR();
		return nullptr;
	}

	RememberedActor* prevTargetAlly = retreatingNPC.m_targetAlly;
	
	RememberedActor* allyToRetreatTo = nullptr;
	int bestDistanceSquared = findClosest ? INT_MAX : -1;
	for (std::vector<Actor*>::const_iterator actorIter = retreatingNPC.m_visibleActors.cbegin(); actorIter != retreatingNPC.m_visibleActors.cend(); ++actorIter){
		Actor* actor = *actorIter;
		FATAL_ASSERT(actor != nullptr);
		RECOVERABLE_ASSERT(actor != &retreatingNPC);

		if (retreatToBuffers){
			if (actor->IsPlayer())
				continue;

			NPC* npc = (NPC*)actor;
			AOEBuffBehavior* buffBehavior = (AOEBuffBehavior*)npc->FindBehaviorByName("AOEBuff");
			if (buffBehavior == nullptr || buffBehavior->m_hasBeenUsed)
				continue;
		}

		if (retreatingNPC.GetFactionStatus(actor->m_faction->m_factionID, actor->m_ID) > 0){ //is ally
			int distanceSquared = CalcDistanceSquared(actor->m_position, retreatingNPC.m_position);
			if ((findClosest && distanceSquared < bestDistanceSquared) || (!findClosest && distanceSquared > bestDistanceSquared)){
				RememberedActor remActor = RememberedActor(*actor);
				retreatingNPC.m_targetAlly = &remActor;

				IntVec2 possibleMoveLocation = retreatToAllyBehavior->CalcNextPathStep();

				if ((possibleMoveLocation.x != -1) && (retreatToBuffers || !findClosest || CalcDistanceSquared(possibleMoveLocation, retreatingNPC.m_targetEnemy->m_position) > CalcDistanceSquared(retreatingNPC.m_position, retreatingNPC.m_targetEnemy->m_position))){
					//only retreat if the first step would take us further away from the enemy in the case of findFurthest
					bestDistanceSquared = distanceSquared;

					if (allyToRetreatTo == nullptr)
						allyToRetreatTo = new RememberedActor(remActor);
					else
						*allyToRetreatTo = remActor;
				}
			}
		}
	}

	if (allyToRetreatTo == nullptr){ //we didn't find anyone to retreat to, so let's see if we remember any nearby allies
		for (std::map<int, RememberedActor>::const_iterator actorIter = retreatingNPC.m_previouslyVisibleActors.cbegin(); actorIter != retreatingNPC.m_previouslyVisibleActors.cend(); ++actorIter){
			const RememberedActor& rememberedActorData = actorIter->second;
			if (Entity::s_turnCount - 5 > rememberedActorData.m_turn)
				continue;

			if (retreatToBuffers && !rememberedActorData.m_canAOEBuff){
				continue;
			}
			
			if (retreatingNPC.GetFactionStatus(rememberedActorData.m_factionID, actorIter->first) > 0){ //is ally
				int distanceSquared = CalcDistanceSquared(rememberedActorData.m_position, retreatingNPC.m_position);
				if ((findClosest && distanceSquared < bestDistanceSquared) || (!findClosest && distanceSquared > bestDistanceSquared)){
					retreatingNPC.m_targetAlly = (RememberedActor*)&rememberedActorData;

					IntVec2 possibleMoveLocation = retreatToAllyBehavior->CalcNextPathStep();

					if ((possibleMoveLocation.x != -1) && (retreatToBuffers || !findClosest || CalcDistanceSquared(possibleMoveLocation, retreatingNPC.m_targetEnemy->m_position) > CalcDistanceSquared(retreatingNPC.m_position, retreatingNPC.m_targetEnemy->m_position))){
						//only retreat if the first step would take us further away from the enemy in the case of findFurthest
						bestDistanceSquared = distanceSquared;

						if (allyToRetreatTo == nullptr)
							allyToRetreatTo = new RememberedActor(rememberedActorData);
						else
							*allyToRetreatTo = rememberedActorData;
					}
				}
			}
		}
	}

	retreatingNPC.m_targetAlly = prevTargetAlly;

	return allyToRetreatTo;
}

///=====================================================
/// 
///=====================================================
void AISystem::DefendStrategy() const{
	static IntVec2 defenceLocation(48, 9);

	for (Actors::const_iterator actorIter = Actor::s_actorsOnMap.cbegin(); actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
		Actor* actor = actorIter->second;

		if (actor->m_isDead)
			continue;

		if (actor->IsPlayer()){
			break;
		}
		else{
			NPC* npc = (NPC*)actor;
			if (npc->m_plannedBehavior->m_name == "Wander" && CalcDistanceSquared(npc->m_position, defenceLocation) > (10 * 10)){ //if we're far away from the defence location and wandering, move toward the defence location
				DefendLocationBehavior* defendLocationBehavior = (DefendLocationBehavior*)npc->FindBehaviorByName("DefendLocation");
				RECOVERABLE_ASSERT(defendLocationBehavior != nullptr);
				if (defendLocationBehavior != nullptr){
					defendLocationBehavior->m_defenceLocation = defenceLocation;
					npc->m_plannedBehavior = defendLocationBehavior;
				}
			}
		}
	}
}
