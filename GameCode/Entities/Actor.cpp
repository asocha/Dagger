//=====================================================
// Actor.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "Actor.hpp"
#include <assert.h>
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../Faction.hpp"
#include "../MessageBar.hpp"
#include "Engine/Core/StringTable.hpp"
#include "NPC.hpp"
#include "../AIBehaviors/AOEBuffBehavior.hpp"

Actors Actor::s_actorsOnMap;

///=====================================================
/// 
///=====================================================
RememberedActor::RememberedActor():
m_isPlayer(false),
m_ID(-1),
m_position(-1, -1),
m_turn(-1),
m_factionID(0),
m_canAOEBuff(false),
m_isFromYellForHelp(false),
m_name(""){
}

///=====================================================
/// 
///=====================================================
RememberedActor::RememberedActor(const Actor& actor) :
m_isPlayer(actor.IsPlayer()),
m_ID(actor.m_ID),
m_position(actor.m_position),
m_turn(Entity::s_turnCount),
m_factionID(0),
m_canAOEBuff(false),
m_isFromYellForHelp(false),
m_name(actor.m_name){
	if (actor.m_faction != nullptr){
		m_factionID = actor.m_faction->m_factionID;
	}

	if (m_isPlayer)
		return;

	const NPC& npc = (const NPC&)actor;
	AOEBuffBehavior* buffBehavior = (AOEBuffBehavior*)npc.FindBehaviorByName("AOEBuff");
	if (buffBehavior == nullptr || buffBehavior->m_hasBeenUsed)
		return;

	m_canAOEBuff = true;
}

///=====================================================
/// 
///=====================================================
Actor* RememberedActor::GetActorFromRememberedActor(const RememberedActor& rememberedActor){
	for (Actors::const_iterator actorIter = Actor::s_actorsOnMap.cbegin(); actorIter != Actor::s_actorsOnMap.cend(); ++actorIter){
		if (actorIter->second->m_ID == rememberedActor.m_ID)
			return actorIter->second;
	}

	return nullptr;
}

///=====================================================
/// 
///=====================================================
Actor::Actor(OpenGLRenderer* renderer) :
Entity(renderer),
m_inventory(nullptr),
m_path(),
m_faction(nullptr),
m_visibleTiles(),
m_visibleActors(),
m_previouslyVisibleActors(),
m_targetEnemy(nullptr),
m_targetAlly(nullptr),
m_chanceToHit(1.0f),
m_isInvulnerable(false),
m_invulnerabilityExpirationTurn(-1),
m_canUseFeatures(false){
}

///=====================================================
/// 
///=====================================================
Actor::Actor(const XMLNode& actorNode, OpenGLRenderer* renderer) :
Entity(actorNode, renderer),
m_inventory(nullptr),
m_path(),
m_faction(nullptr),
m_visibleTiles(),
m_visibleActors(),
m_previouslyVisibleActors(),
m_targetEnemy(nullptr),
m_targetAlly(nullptr),
m_chanceToHit(1.0f),
m_isInvulnerable(false),
m_invulnerabilityExpirationTurn(-1),
m_canUseFeatures(false){
	m_faction = Faction::GetNewFactionByName(GetStringAttribute(actorNode, "faction"));
	m_canUseFeatures = GetBoolAttribute(actorNode, "canUseFeatures", m_canUseFeatures);
}

///=====================================================
/// 
///=====================================================
Actor::Actor(const Actor& other, OpenGLRenderer* renderer, const XMLNode& saveData /*= XMLNode::emptyNode()*/):
Entity(other, renderer, saveData),
m_inventory(nullptr),
m_path(),
m_faction(nullptr),
m_visibleTiles(),
m_visibleActors(),
m_previouslyVisibleActors(),
m_targetEnemy(nullptr),
m_targetAlly(nullptr),
m_chanceToHit(other.m_chanceToHit),
m_isInvulnerable(false),
m_invulnerabilityExpirationTurn(-1),
m_canUseFeatures(other.m_canUseFeatures){
	if (other.m_inventory != nullptr)
		m_inventory = new Inventory(*other.m_inventory);
	if (other.m_faction != nullptr){
		m_faction = new Faction(*other.m_faction);
	}

	if (!saveData.isEmpty()){
		std::string factionName = GetStringAttribute(saveData, "faction", GetStringFromID(m_faction->m_factionID));
		if (!factionName.empty()){
			delete m_faction;

			XMLNode relationshipsNode = saveData.getChildNode("Relationships");
			m_faction = Faction::GetNewFactionByName(factionName, relationshipsNode);
		}

		XMLNode inventoryNode = saveData.getChildNode("Inventory");
		if (!inventoryNode.isEmpty()){
			delete m_inventory;
			m_inventory = new Inventory(inventoryNode);
		}

		m_targetEnemy = (RememberedActor*)GetIntAttribute(saveData, "targetEnemy", NULL);
		m_targetAlly = (RememberedActor*)GetIntAttribute(saveData, "targetAlly", NULL);
	}
}

///=====================================================
/// 
///=====================================================
Actor::~Actor(){
	if (m_inventory != nullptr){
		delete m_inventory;
		m_inventory = nullptr;
	}

	if (m_faction != nullptr){
		delete m_faction;
		m_faction = nullptr;
	}

	if (m_targetEnemy != nullptr){
		delete m_targetEnemy;
		m_targetEnemy = nullptr;
	}

	if (m_targetAlly != nullptr){
		delete m_targetAlly;
		m_targetAlly = nullptr;
	}
}

///=====================================================
/// 
///=====================================================
void Actor::AddToMap(Map* map, const IntVec2& position){
	assert(map != nullptr);
	m_map = map;
	m_position = position;

	Tile& tile = map->GetTileAtPosition(position);
	assert(tile.CanBeMovedInto());
	tile.m_actor = this;


	float nextSpeed;
	Actors::const_iterator firstActorIter = s_actorsOnMap.cbegin();
	if (firstActorIter != s_actorsOnMap.cend()){
		nextSpeed = firstActorIter->first;
		if (firstActorIter->second->IsPlayer())
			nextSpeed += 0.5f;
	}
	else{
		nextSpeed = 1.0f;
	}

	if (IsPlayer())
		nextSpeed -= 0.5f;

	s_actorsOnMap.emplace(nextSpeed, this);
}

///=====================================================
/// 
///=====================================================
bool Actor::CanMoveOneStep(const IntVec2& direction) const{
	const Tile& endTile = m_map->GetTileAtPosition(m_position + direction);
	if (endTile.CanBeMovedInto())
		return true;
	return false;
}

///=====================================================
/// 
///=====================================================
void Actor::MoveOneStep(const IntVec2& direction){
	assert(CanMoveOneStep(direction));

	Tile& startTile = m_map->GetTileAtPosition(m_position);
	startTile.m_actor = nullptr;
	m_position += direction;

	Tile& endTile = m_map->GetTileAtPosition(m_position);
	endTile.m_actor = this;

	m_path.Reset();
}

///=====================================================
/// 
///=====================================================
bool Actor::CanMoveToLocation(const IntVec2& location) const{
	const Tile& endTile = m_map->GetTileAtPosition(location);
	if (endTile.CanBeMovedInto() && m_visibleTiles.find((Tile*)&endTile) != m_visibleTiles.cend())
		return true;
	return false;
}

///=====================================================
/// 
///=====================================================
void Actor::MoveToLocation(const IntVec2& location){
	assert(CanMoveToLocation(location) || m_name == "Vampire Lord");

	Tile& startTile = m_map->GetTileAtPosition(m_position);
	startTile.m_actor = nullptr;
	m_position = location;

	Tile& endTile = m_map->GetTileAtPosition(m_position);
	endTile.m_actor = this;

	m_path.Reset();
}

///=====================================================
/// 
///=====================================================
void Actor::PickUpItem(){
	Tile& tile = m_map->GetTileAtPosition(m_position);
	if (tile.m_items == nullptr){
		return;
	}

	std::multimap<Item::ItemType, Item*>& items = tile.m_items->m_backpack;
	if (items.empty()){
		return;
	}

	if (m_inventory == nullptr)
		m_inventory = new Inventory();

	std::multimap<Item::ItemType, Item*>::iterator firstItemIter = items.begin();
	Item* item = firstItemIter->second;
	assert(item != nullptr);

	items.erase(firstItemIter);
	if (items.empty()){
		delete tile.m_items;
		tile.m_items = nullptr;
	}

	item->m_position = IntVec2(-1, -1);

	Item::EquipmentSlot slot = item->m_equippedSlot;
	if (slot != Item::Unequippable){
		//NPCs gain bravery for each slot they have an item equipped in
		if (!IsPlayer() && (m_inventory->m_equipment.find(slot) != m_inventory->m_equipment.cend())){
			NPC* thisNPC = (NPC*)this;
			if (slot == Item::EquipmentSlot::WeaponSlot)
				thisNPC->m_bravery += 0.2f;
			else
				thisNPC->m_bravery += 0.1f;
		}
		m_inventory->AddToEquipment(*item);
	}
	else{
		m_inventory->AddToBackpack(*item);
	}


	for (Items::const_iterator itemIter = Item::s_itemsOnMap.cbegin(); itemIter != Item::s_itemsOnMap.cend(); ++itemIter){
		Item* mapItem = *itemIter;
		if (mapItem == item){
			Item::s_itemsOnMap.erase(itemIter);
			break;
		}
	}

	if (IsPlayer() || CanSeePlayer()){
		s_theMessageBar->m_messages.push_back(m_name + " picked up a " + item->m_name + "!");
	}	
}

///=====================================================
/// only used by player for testing
///=====================================================
void Actor::PathfindToDestination(const IntVec2& goal, bool doActorsBlock, bool calcFullPath /*= true*/){
	m_path.CalcPath(false, this, m_position, goal, m_map, doActorsBlock, calcFullPath);
}

///=====================================================
/// 
///=====================================================
bool Actor::CanSeePlayer() const{
	for (std::vector<Actor*>::const_iterator visibleActorIter = m_visibleActors.cbegin(); visibleActorIter != m_visibleActors.cend(); ++visibleActorIter){
		if ((*visibleActorIter)->IsPlayer()){
			return true;
		}
	}

	return false;
}

///=====================================================
/// 
///=====================================================
void Actor::Render(OpenGLRenderer* renderer, bool forceVisible /*= false*/){
	Entity::Render(renderer, forceVisible);

	//render our path for debugging purposes
	if (m_path.m_goal.x == -1) return;

	m_textRenderer.SetInputStringColor(RGBAchars::GREEN);
	m_textRenderer.SetInputString("X");
	m_textRenderer.RenderText(renderer, Font::CONSOLAS, m_map->m_fontSize, m_map->m_renderOffset + Vec2(103.0f + (float)m_path.m_goal.x * (16.4f / 35.0f) * m_map->m_fontSize, 100.0f + (float)m_path.m_goal.y * m_map->m_fontSize));

	m_textRenderer.SetInputStringColor(RGBAchars::PURPLE);
	m_textRenderer.SetInputString("*");
	for (std::deque<PathNode*>::const_iterator pathIter = m_path.m_path.begin(); pathIter != m_path.m_path.cend(); ++pathIter){
		PathNode* pathNode = *pathIter;

		m_textRenderer.RenderText(renderer, Font::CONSOLAS, m_map->m_fontSize, m_map->m_renderOffset + Vec2(103.0f + (float)pathNode->m_position.x * (16.4f / 35.0f) * m_map->m_fontSize, 100.0f + (float)pathNode->m_position.y * m_map->m_fontSize));
	}

	m_textRenderer.SetInputStringColor(RGBAchars::BLUE);
	for (std::multimap<float, PathNode*>::const_iterator pathIter = m_path.m_openList.begin(); pathIter != m_path.m_openList.cend(); ++pathIter){
		PathNode* pathNode = pathIter->second;

		m_textRenderer.RenderText(renderer, Font::CONSOLAS, m_map->m_fontSize, m_map->m_renderOffset + Vec2(103.0f + (float)pathNode->m_position.x * (16.4f / 35.0f) * m_map->m_fontSize, 100.0f + (float)pathNode->m_position.y * m_map->m_fontSize));
	}

	m_textRenderer.SetInputStringColor(RGBAchars::RED);
	for (std::map<IntVec2, PathNode*>::const_iterator pathIter = m_path.m_closedList.begin(); pathIter != m_path.m_closedList.cend(); ++pathIter){
		PathNode* pathNode = pathIter->second;

		m_textRenderer.RenderText(renderer, Font::CONSOLAS, m_map->m_fontSize, m_map->m_renderOffset + Vec2(103.0f + (float)pathNode->m_position.x * (16.4f / 35.0f) * m_map->m_fontSize, 100.0f + (float)pathNode->m_position.y * m_map->m_fontSize));
	}

	m_textRenderer.SetInputStringColor(RGBAchars::RED);
	m_textRenderer.SetInputString(std::string(1, m_glyph));
}

///=====================================================
/// 
///=====================================================
void Actor::Die(){
	m_map->GetTileAtPosition(m_position).m_actor = nullptr;

	//drop all items
	if (m_inventory != nullptr){
		Tile& tile = m_map->GetTileAtPosition(m_position);
		if (tile.m_items == nullptr)
			tile.m_items = new Inventory();

		for (std::multimap<Item::ItemType, Item*>::iterator itemIter = m_inventory->m_backpack.begin(); itemIter != m_inventory->m_backpack.end(); ++itemIter){
			Item* item = itemIter->second;
			assert(item != nullptr);

			item->AddToMap(m_map, m_position);
		}

		for (std::map<Item::EquipmentSlot, Item*>::iterator itemIter = m_inventory->m_equipment.begin(); itemIter != m_inventory->m_equipment.end(); ++itemIter){
			Item* item = itemIter->second;
			assert(item != nullptr);

			item->AddToMap(m_map, m_position);
		}

		m_inventory->m_backpack.clear();
		m_inventory->m_equipment.clear();

		delete m_inventory;
		m_inventory = nullptr;
	}

	Entity::Die();
}

///=====================================================
/// 
///=====================================================
void Actor::ReactToDeadActor(const Actor& actor){
	for (std::vector<Actor*>::iterator visibleActorIter = m_visibleActors.begin(); visibleActorIter != m_visibleActors.end(); ++visibleActorIter){
		Actor* visibleActor = *visibleActorIter;
		if (visibleActor == &actor){
			m_visibleActors.erase(visibleActorIter);

			std::map<int, RememberedActor>::const_iterator previouslyVisibleActorIter = m_previouslyVisibleActors.find(actor.m_ID);
			if (previouslyVisibleActorIter != m_previouslyVisibleActors.cend())
				m_previouslyVisibleActors.erase(previouslyVisibleActorIter);

			if (m_targetEnemy != nullptr && m_targetEnemy->m_ID == actor.m_ID){
				delete m_targetEnemy;
				m_targetEnemy = nullptr;
			}
			else if (m_targetAlly != nullptr && m_targetAlly->m_ID == actor.m_ID){
				delete m_targetAlly;
				m_targetAlly = nullptr;
			}

			break;
		}
	}
}

///=====================================================
/// 
///=====================================================
int Actor::GetFactionStatus(unsigned int factionID, int entityID) const{
	const EntityRelationshipMap& entityRelMap = m_faction->m_entityRelationshipMap;
	EntityRelationshipMap::const_iterator entityRelIter = entityRelMap.find(entityID);
	if (entityRelIter != entityRelMap.cend()){
		return entityRelIter->second;
	}
	else{
		if (factionID == m_faction->m_factionID){ //member of our own faction
			return FactionStatus::Loved;
		}

		const FactionRelationshipMap& factionRelMap = m_faction->m_factionRelationshipMap;
		FactionRelationshipMap::const_iterator factionRelIter = factionRelMap.find(factionID);
		if (factionRelIter != factionRelMap.cend()){
			return factionRelIter->second;
		}
	}

	assert(false);
	return FactionStatus::Neutral;
}

///=====================================================
/// 
///=====================================================
void Actor::AdjustFactionStatus(unsigned int factionID, int adjustment, int entityID){
	EntityRelationshipMap& entityRelMap = m_faction->m_entityRelationshipMap;
	EntityRelationshipMap::iterator entityRelIter = entityRelMap.find(entityID);
	if (entityRelIter != entityRelMap.end()){
		int entityRelStatus = entityRelIter->second;
		entityRelStatus += adjustment;
		entityRelIter->second = (FactionStatus)entityRelStatus;
	}
	else{
		entityRelMap.emplace(entityID, (FactionStatus)adjustment);
	}

	if (factionID == m_faction->m_factionID) //don't adjust relationship to our own faction
		return;

	FactionRelationshipMap& factionRelMap = m_faction->m_factionRelationshipMap;
	FactionRelationshipMap::iterator factionRelIter = factionRelMap.find(factionID);
	if (factionRelIter != factionRelMap.end()){
		int factionRelStatus = factionRelIter->second;
		factionRelStatus += adjustment;
		factionRelIter->second = (FactionStatus)factionRelStatus;
	}
	else{
		assert(false);
	}
}

///=====================================================
/// 
///=====================================================
void Actor::SaveGame(XMLNode& actorNode) const{
	if (m_inventory != nullptr){
		XMLNode inventoryNode = actorNode.addChild("Inventory");
		m_inventory->SaveGame(inventoryNode);
	}

	if (m_faction != nullptr){
		SetStringAttribute(actorNode, "faction", GetStringFromID(m_faction->m_factionID));
		XMLNode relationshipsNode = actorNode.addChild("Relationships");
		m_faction->SaveGame(relationshipsNode);
	}

	if (m_targetEnemy != nullptr)
		SetIntAttribute(actorNode, "targetEnemy", m_targetEnemy->m_ID);

	if (m_targetAlly != nullptr)
		SetIntAttribute(actorNode, "targetAlly", m_targetAlly->m_ID);
}

///=====================================================
/// 
///=====================================================
void Actor::ResolvePointersFromLoadGame(){
	if (m_targetEnemy != nullptr)
		m_targetEnemy = new RememberedActor(*(Actor*)s_loadGameLinkMap[(int)m_targetEnemy]);

	if (m_targetAlly != nullptr)
		m_targetAlly = new RememberedActor(*(Actor*)s_loadGameLinkMap[(int)m_targetAlly]);

	if (m_faction != nullptr)
		m_faction->ResolvePointersFromLoadGame();
}
