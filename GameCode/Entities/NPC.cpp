//=====================================================
// NPC.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "NPC.hpp"
#include "../AIBehaviors/BaseAIBehavior.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../ViewFinding.hpp"
#include "../Tile.hpp"
#include "../Faction.hpp"
#include "../MessageBar.hpp"
#include "Entity.hpp"
#include "../Map.hpp"
#include "Player.hpp"

int NPC::s_mostRecentYellForHelpDueToPlayerTurn = -1;

///=====================================================
/// 
///=====================================================
NPC::NPC(const XMLNode& npcNode, OpenGLRenderer* renderer) :
Actor(npcNode, renderer),
m_behaviors(),
m_plannedBehavior(nullptr),
m_ignoredBehaviors(),
m_ignoreRetreatStrategy(false),
m_bravery(1.0f),
m_yelledForHelp(false),
m_canYellForHelp(false){
	XMLNode behaviorNode = npcNode.getChildNode("AIBehavior");
	for (int i = 1; !behaviorNode.isEmpty(); ++i){
		std::string behaviorName = GetStringAttribute(behaviorNode, "name");
		BehaviorRegistry::const_iterator behaviorIter = AIBehaviorRegistration::GetBehaviorRegistry()->find(behaviorName);
		RECOVERABLE_ASSERT(behaviorIter != AIBehaviorRegistration::GetBehaviorRegistry()->cend());

		if (behaviorIter != AIBehaviorRegistration::GetBehaviorRegistry()->cend()){
			BaseAIBehavior* newBehavior = behaviorIter->second->CreateNewAIBehavior(behaviorName, behaviorNode);
			newBehavior->m_NPC = this;
			m_behaviors.push_back(newBehavior);
		}

		behaviorNode = npcNode.getChildNode("AIBehavior", i);
	}

	m_bravery = GetFloatAttribute(npcNode, "bravery", m_bravery);
	m_chanceToHit = GetFloatAttribute(npcNode, "chanceToHit", m_chanceToHit);
	m_canYellForHelp = GetBoolAttribute(npcNode, "canYellForHelp", m_canYellForHelp);

	m_glyph = GetCharAttribute(npcNode, "glyph", m_glyph);
	m_color = GetColorAttribute(npcNode, "color", m_color);

	m_textRenderer.SetInputString(std::string(1, m_glyph));
	m_textRenderer.SetInputStringColor(m_color);
}

///=====================================================
/// 
///=====================================================
NPC::NPC(const NPC& other, OpenGLRenderer* renderer, const XMLNode& saveData /*= XMLNode::emptyNode()*/) :
Actor(other, renderer, saveData),
m_behaviors(),
m_plannedBehavior(nullptr),
m_ignoredBehaviors(),
m_ignoreRetreatStrategy(false),
m_bravery(other.m_bravery),
m_yelledForHelp(other.m_yelledForHelp),
m_canYellForHelp(other.m_canYellForHelp){
	if (!saveData.isEmpty()){
		m_ignoreRetreatStrategy = GetBoolAttribute(saveData, "ignoreRetreat", m_ignoreRetreatStrategy);
		m_bravery = GetFloatAttribute(saveData, "bravery", m_bravery);
		m_yelledForHelp = GetBoolAttribute(saveData, "yelledForHelp", m_yelledForHelp);
	}

	//duplicate other's behaviors
	for (std::vector<BaseAIBehavior*>::const_iterator behaviorIter = other.m_behaviors.cbegin(); behaviorIter != other.m_behaviors.cend(); ++behaviorIter){
		const BaseAIBehavior* behavior = *behaviorIter;
		BaseAIBehavior* newBehavior = (AIBehaviorRegistration::GetBehaviorRegistry()->at(behavior->m_name))->CreateNewAIBehavior(behavior->m_name, behavior->m_behaviorNode);
		newBehavior->m_NPC = this;
		m_behaviors.push_back(newBehavior);
	}

	m_textRenderer.SetInputString(std::string(1, m_glyph));
	m_textRenderer.SetInputStringColor(m_color);
}

///=====================================================
/// 
///=====================================================
NPC::~NPC(){
	for (std::vector<BaseAIBehavior*>::const_iterator behaviorIter = m_behaviors.cbegin(); behaviorIter != m_behaviors.cend(); ++behaviorIter){
		delete *behaviorIter;
	}
}

///=====================================================
/// 
///=====================================================
bool NPC::Think(bool isMainThink){
	if (isMainThink){
		if (m_isDead)
			return true;

		if (m_isInvulnerable && m_invulnerabilityExpirationTurn == s_turnCount){
			m_isInvulnerable = false;
			m_bravery -= 1.0f;
		}
	}

	if (m_plannedBehavior == nullptr)
		return false;

	bool didThink = m_plannedBehavior->Think();

	m_ignoredBehaviors.clear();

	return didThink;
}

///=====================================================
/// 
///=====================================================
void NPC::PlanNextThink(bool recomputeFieldOfView/* = true*/){
	if (m_isDead)
		return;

	if (recomputeFieldOfView){
		//determine visible actors and target enemy
		m_visibleActors.clear();
		m_visibleTiles = CalculateFieldOfView(m_map, m_position);
		delete m_targetEnemy;
		m_targetEnemy = nullptr;
		delete m_targetAlly;
		m_targetAlly = nullptr;

		int closestEnemyDistanceSquared = INT_MAX;
		int closestAllyDistanceSquared = INT_MAX;
		for (std::set<Tile*>::const_iterator visibleTileIter = m_visibleTiles.cbegin(); visibleTileIter != m_visibleTiles.cend(); ++visibleTileIter){
			Actor* actor = (*visibleTileIter)->m_actor;

			if (actor != nullptr && actor != this && !actor->m_isDead){
				m_visibleActors.push_back(actor);

				int factionStatus = GetFactionStatus(actor->m_faction->m_factionID, actor->m_ID);
				if (factionStatus < 0){ //is enemy
					int distanceSquared = CalcDistanceSquared(actor->m_position, m_position);
					if (distanceSquared < closestEnemyDistanceSquared){
						closestEnemyDistanceSquared = distanceSquared;

						if (m_targetEnemy == nullptr)
							m_targetEnemy = new RememberedActor(*actor);
						else
							new(m_targetEnemy) RememberedActor(*actor);
					}
				}
				else if (factionStatus > 0){
					int distanceSquared = CalcDistanceSquared(actor->m_position, m_position);
					if (distanceSquared < closestAllyDistanceSquared){
						closestAllyDistanceSquared = distanceSquared;

						if (m_targetAlly == nullptr)
							m_targetAlly = new RememberedActor(*actor);
						else
							new(m_targetAlly) RememberedActor(*actor);
					}
				}
			}
		}

		if (m_targetEnemy == nullptr){ //if we didn't find a target, check our remembered actors for a target
			for (std::map<int, RememberedActor>::const_iterator actorIter = m_previouslyVisibleActors.cbegin(); actorIter != m_previouslyVisibleActors.cend(); ++actorIter){
				const RememberedActor& rememberedActorData = actorIter->second;
				if (rememberedActorData.m_isFromYellForHelp){
					if (Entity::s_turnCount - 15 > rememberedActorData.m_turn)
						continue;
				}
				else{
					if (Entity::s_turnCount - 5 > rememberedActorData.m_turn)
						continue;
				}

				if (GetFactionStatus(rememberedActorData.m_factionID, actorIter->first) < 0){ //is enemy
					int distanceSquared = CalcDistanceSquared(rememberedActorData.m_position, m_position);
					if (distanceSquared < closestEnemyDistanceSquared){
						closestEnemyDistanceSquared = distanceSquared;

						if (m_targetEnemy == nullptr)
							m_targetEnemy = new RememberedActor(rememberedActorData);
						else
							*m_targetEnemy = rememberedActorData;
					}
				}
			}
		}

		if (m_targetAlly == nullptr){ //if we didn't find a target, check our remembered actors for a target
			for (std::map<int, RememberedActor>::const_iterator actorIter = m_previouslyVisibleActors.cbegin(); actorIter != m_previouslyVisibleActors.cend(); ++actorIter){
				const RememberedActor& rememberedActorData = actorIter->second;
				if (rememberedActorData.m_isFromYellForHelp){
					if (Entity::s_turnCount - 15 > rememberedActorData.m_turn)
						continue;
				}
				else{
					if (Entity::s_turnCount - 5 > rememberedActorData.m_turn)
						continue;
				}

				if (GetFactionStatus(rememberedActorData.m_factionID, actorIter->first) > 0){ //is ally
					int distanceSquared = CalcDistanceSquared(rememberedActorData.m_position, m_position);
					if (distanceSquared < closestAllyDistanceSquared){
						closestAllyDistanceSquared = distanceSquared;

						if (m_targetAlly == nullptr)
							m_targetAlly = new RememberedActor(rememberedActorData);
						else
							*m_targetAlly = rememberedActorData;
					}
				}
			}
		}

		//remember the locations of previously visible actors
		for (std::vector<Actor*>::const_iterator visibleActorIter = m_visibleActors.cbegin(); visibleActorIter != m_visibleActors.cend(); ++visibleActorIter){
			Actor* actor = *visibleActorIter;
			FATAL_ASSERT(actor != nullptr);

			RememberedActor actorInfo(*actor);
			m_previouslyVisibleActors[actor->m_ID] = actorInfo;
		}
	}

	BaseAIBehavior* bestBehavior = nullptr;
	float bestUtility = 0.0f;
	for (std::vector<BaseAIBehavior*>::const_iterator behaviorIter = m_behaviors.cbegin(); behaviorIter != m_behaviors.cend(); ++behaviorIter){
		for (std::vector<BaseAIBehavior*>::const_iterator ignoredBehaviorIter = m_ignoredBehaviors.cbegin(); ignoredBehaviorIter != m_ignoredBehaviors.cend(); ++ignoredBehaviorIter){
			if (*behaviorIter == *ignoredBehaviorIter)
				goto ignoreBehavior;
		}

		float utility = (*behaviorIter)->CalcUtility();
		if (utility > bestUtility){
			bestUtility = utility;
			bestBehavior = *behaviorIter;
		}

ignoreBehavior:;
	}

	m_plannedBehavior = bestBehavior;
}

///=====================================================
/// 
///=====================================================
BaseAIBehavior* NPC::FindBehaviorByName(const std::string& name) const{
	for (std::vector<BaseAIBehavior*>::const_iterator behaviorIter = m_behaviors.cbegin(); behaviorIter != m_behaviors.cend(); ++behaviorIter){
		if ((*behaviorIter)->m_name == name)
			return *behaviorIter;
	}

	return nullptr;
}

///=====================================================
/// 
///=====================================================
void NPC::TakeDamage(int damage, const Actor* attacker){
	Entity::TakeDamage(damage, attacker);

	if (damage > 0 && m_visibleActors.size() <= 2){
		if (m_health > 0 && m_bravery < 2.0f && ((float)m_health < 8.0f - 2.0f * m_bravery)){
			YellForHelp(attacker);
		}
	}
}

///=====================================================
/// 
///=====================================================
void NPC::YellForHelp(const Actor* attacker){
	if (m_yelledForHelp || !m_canYellForHelp)
		return;

	IntVec2 yellPosition = m_position + IntVec2(GetRandomIntInRange(-1, 1), GetRandomIntInRange(-1, 1));
	while (m_map->GetTileAtPosition(yellPosition).GetTileType() != TileType::Air)
		yellPosition = m_position + IntVec2(GetRandomIntInRange(-1, 1), GetRandomIntInRange(-1, 1));

	int yellLoudness = GetRandomIntInRange(12, 20);

	Player* thePlayer = nullptr;

	for (Actors::const_iterator actorIter = Actor::s_actorsOnMap.cbegin(); actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
		Actor& actor = *actorIter->second;
		if (&actor == this)
			continue;

		if (actor.IsPlayer()){
			thePlayer = (Player*)&actor;
			continue;
		}

		if (CalcDistanceSquared(m_position, actor.m_position) > (yellLoudness * yellLoudness))
			continue;

		//everyone learns about the yelling NPC
		std::map<int, RememberedActor>::iterator rememberedNPCIter = actor.m_previouslyVisibleActors.find(m_ID);
		if (rememberedNPCIter != actor.m_previouslyVisibleActors.end()){
			if (rememberedNPCIter->second.m_turn < Entity::s_turnCount - 1){
				rememberedNPCIter->second.m_position = yellPosition;
				rememberedNPCIter->second.m_turn = Entity::s_turnCount;
			}

			rememberedNPCIter->second.m_isFromYellForHelp = true;
		}
		else{
			RememberedActor rememberedNPC(*this);
			rememberedNPC.m_position = yellPosition;
			rememberedNPC.m_canAOEBuff = false;
			rememberedNPC.m_isFromYellForHelp = true;

			actor.m_previouslyVisibleActors.emplace(m_ID, rememberedNPC);
		}

		//everyone learns about the attacker
		if (attacker != nullptr){
			std::map<int, RememberedActor>::iterator rememberedAttackerIter = actor.m_previouslyVisibleActors.find(attacker->m_ID);
			if (rememberedAttackerIter != actor.m_previouslyVisibleActors.end()){
				if (rememberedAttackerIter->second.m_turn < Entity::s_turnCount - 1){
					rememberedAttackerIter->second.m_position = yellPosition;
					rememberedAttackerIter->second.m_turn = Entity::s_turnCount;
				}

				rememberedAttackerIter->second.m_isFromYellForHelp = true;
			}
			else{
				RememberedActor rememberedAttacker(*attacker);
				rememberedAttacker.m_position = yellPosition;
				rememberedAttacker.m_canAOEBuff = false;
				rememberedAttacker.m_isFromYellForHelp = true;

				actor.m_previouslyVisibleActors.emplace(attacker->m_ID, rememberedAttacker);
			}
		}
	}

	m_yelledForHelp = true;

	if (attacker->IsPlayer())
		s_mostRecentYellForHelpDueToPlayerTurn = Entity::s_turnCount;

	if (attacker->IsPlayer() || CanSeePlayer())
		s_theMessageBar->m_messages.push_back(m_name + " yells for help!");
	else if (CalcDistanceSquared(yellPosition, thePlayer->m_position) <= yellLoudness){
		std::string myNameToLower = m_name;
		std::transform(m_name.begin(), m_name.end(), myNameToLower.begin(), ::tolower);
		s_theMessageBar->m_messages.push_back("You hear a " + myNameToLower + " yell for help in the distance!");
	}
}

///=====================================================
/// 
///=====================================================
void NPC::SaveGame(XMLNode& entitiesNode) const{
	XMLNode npcNode = entitiesNode.addChild("NPC");
	Entity::SaveGame(npcNode);
	Actor::SaveGame(npcNode);

	SetBoolAttribute(npcNode, "ignoreRetreat", m_ignoreRetreatStrategy, false);
	SetFloatAttribute(npcNode, "bravery", m_bravery, 1.0f);
	SetBoolAttribute(npcNode, "yelledForHelp", m_yelledForHelp, false);
}
