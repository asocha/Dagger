//=====================================================
// Player.cpp
// by Andrew Socha
//=====================================================

#include "Player.hpp"
#include "../Map.hpp"
#include "../ViewFinding.hpp"
#include "../CombatSystem.hpp"
#include "../Faction.hpp"
#include "../MessageBar.hpp"
#include "Feature.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"

///=====================================================
/// 
///=====================================================
Player::Player(OpenGLRenderer* renderer) :
Actor(renderer),
m_killCount(0){
	s_turnCount = 0;

	m_glyph = '@';
	m_health = 150;
	m_maxHealth = 150;
	m_name = "You";
	m_canUseFeatures = true;

	m_faction = Faction::GetNewFactionByName("Player");

	m_color = RGBAchars::RED;
	m_textRenderer.SetInputString(std::string(1, m_glyph));
	m_textRenderer.SetInputStringColor(m_color);
}

///=====================================================
/// 
///=====================================================
Player::Player(const Player& other, OpenGLRenderer* renderer, const XMLNode& playerNode) :
Actor(other, renderer, playerNode),
m_killCount(0){
	s_turnCount = 0;
	s_turnCount = GetIntAttribute(playerNode, "turnCount", s_turnCount);
	m_killCount = GetIntAttribute(playerNode, "killCount", m_killCount);

	m_textRenderer.SetInputString(std::string(1, m_glyph));
	m_textRenderer.SetInputStringColor(m_color);
}

///=====================================================
/// 
///=====================================================
bool Player::Think(bool /*isMainThink*/){
	FATAL_ASSERT(IsReadyToThink());

	if (m_isInvulnerable && m_invulnerabilityExpirationTurn == s_turnCount){
		m_isInvulnerable = false;
	}

	s_theMessageBar->m_text.clear();
	UpdateMovement();
	++s_turnCount;
	return true;
}

///=====================================================
/// 0x6X keys are numpad movement
///=====================================================
void Player::UpdateMovement(){
	if (s_theInputSystem->GetKeyWentDown('K') || s_theInputSystem->GetKeyWentDown(0x68)){
		IntVec2 direction(0, 1);
		MoveOrAttackInDirection(direction);
	}
	else if (s_theInputSystem->GetKeyWentDown('J') || s_theInputSystem->GetKeyWentDown(0x62)){
		IntVec2 direction(0, -1);
		MoveOrAttackInDirection(direction);
	}
	else if (s_theInputSystem->GetKeyWentDown('L') || s_theInputSystem->GetKeyWentDown(0x66)){
		IntVec2 direction(1, 0);
		MoveOrAttackInDirection(direction);
	}
	else if (s_theInputSystem->GetKeyWentDown('H') || s_theInputSystem->GetKeyWentDown(0x64)){
		IntVec2 direction(-1, 0);
		MoveOrAttackInDirection(direction);
	}
	else if (s_theInputSystem->GetKeyWentDown('U') || s_theInputSystem->GetKeyWentDown(0x69)){
		IntVec2 direction(1, 1);
		MoveOrAttackInDirection(direction);
	}
	else if (s_theInputSystem->GetKeyWentDown('Y') || s_theInputSystem->GetKeyWentDown(0x67)){
		IntVec2 direction(-1, 1);
		MoveOrAttackInDirection(direction);
	}
	else if (s_theInputSystem->GetKeyWentDown('B') || s_theInputSystem->GetKeyWentDown(0x61)){
		IntVec2 direction(-1, -1);
		MoveOrAttackInDirection(direction);
	}
	else if (s_theInputSystem->GetKeyWentDown('N') || s_theInputSystem->GetKeyWentDown(0x63)){
		IntVec2 direction(1, -1);
		MoveOrAttackInDirection(direction);
	}
	else if (s_theInputSystem->GetCurrentCharacterDown() == ','){ //pick up an item
		PickUpItem();
	}
	else if (s_theInputSystem->GetKeyWentDown('A')){ //use a nearby feature
		const std::vector<Tile*>& nearbyTiles = m_map->GetTilesWithinRadius(m_position);
		for (std::vector<Tile*>::const_iterator tileIter = nearbyTiles.cbegin(); tileIter != nearbyTiles.cend(); ++tileIter){
			Feature* feature = (*tileIter)->m_feature;
			if (feature != nullptr && !feature->m_isUsed){
				feature->UseFeature(*this);
				UpdateMapVisibility();
				return;
			}
		}
	}
	else if (s_theInputSystem->GetKeyWentDown('Q')){ //drink potion
		if (m_health == m_maxHealth)
			return;

		if (m_inventory == nullptr)
			return;

		for (Backpack::iterator itemIter = m_inventory->m_backpack.begin(); itemIter != m_inventory->m_backpack.end(); ++itemIter){
			if (itemIter->first == Item::Consumable){
				Item* item = itemIter->second;
				int tempHealth = m_health;
				m_health += item->m_healing;
				m_health = m_health > m_maxHealth ? m_maxHealth : m_health;
				int healing = m_health - tempHealth;

				s_theMessageBar->m_messages.push_back(item->m_name + " restored " + std::to_string(healing) + " health.");

				m_inventory->RemoveFromBackpack(*item);
				item->Die();

				return;
			}
		}
	}
}

///=====================================================
/// 
///=====================================================
void Player::MoveOrAttackInDirection(const IntVec2& direction){
	const Tile& destinationTile = m_map->GetTileAtPosition(m_position + direction);
	if (CanMoveOneStep(direction)){
		MoveOneStep(direction);
		UpdateMapVisibility();

		//check if we walked over an item
		if (destinationTile.m_items != nullptr){
			const std::multimap<Item::ItemType, Item*>& items = destinationTile.m_items->m_backpack;
			for (std::multimap<Item::ItemType, Item*>::const_iterator itemIter = items.cbegin(); itemIter != items.cend(); ++itemIter){
				s_theMessageBar->m_messages.push_back("You found a " + itemIter->second->m_name + "!");
			}
		}
	}
	else{
		Actor* actor = destinationTile.m_actor;
		if (actor != nullptr && actor->m_faction != nullptr && GetFactionStatus(actor->m_faction->m_factionID, actor->m_ID) < 0){
			AttackData meleeAttackData;
			meleeAttackData.m_attacker = this;
			meleeAttackData.m_target = actor;
			meleeAttackData.m_damageRange = IntVec2(5, 10);
			meleeAttackData.m_attackName = "stabbed";
			meleeAttackData.m_hitChance = m_chanceToHit;

			CombatSystem::PerformAttack(meleeAttackData);

			if (meleeAttackData.m_didKill)
				++m_killCount;
		}
	}
}

///=====================================================
/// 
///=====================================================
void Player::UpdateMapVisibility(){
	//mark all tiles as not visible
	for (std::vector<Tile>::iterator tileIter = m_map->m_tiles.begin(); tileIter != m_map->m_tiles.end(); ++tileIter){
		tileIter->m_isVisibleToPlayer = false;
	}

	//determine what tiles we can see
	m_visibleTiles = CalculateFieldOfView(m_map, m_position);
	for (std::set<Tile*>::iterator visibleTileIter = m_visibleTiles.begin(); visibleTileIter != m_visibleTiles.end(); ++visibleTileIter){
		(*visibleTileIter)->m_isVisibleToPlayer = true;
		(*visibleTileIter)->m_hasBeenSeen = true;
	}
}

///=====================================================
/// 
///=====================================================
void Player::AddToMap(Map* map, const IntVec2& position){
	FATAL_ASSERT(map != nullptr);
	Actor* existingActor =  map->GetTileAtPosition(position).m_actor;
	if (existingActor != nullptr){
		existingActor->Die();
	}

	Feature* existingFeature = map->GetTileAtPosition(position).m_feature;
	if (existingFeature != nullptr){
		existingFeature->Die();
	}

	Actor::AddToMap(map, position);

	UpdateMapVisibility();
}

///=====================================================
/// 
///=====================================================
void Player::SaveGame(XMLNode& entitiesNode) const{
	XMLNode playerNode = entitiesNode.addChild("Player");
	Entity::SaveGame(playerNode);
	Actor::SaveGame(playerNode);

	SetIntAttribute(playerNode, "turnCount", s_turnCount);
	SetIntAttribute(playerNode, "killCount", m_killCount, 0);
}
