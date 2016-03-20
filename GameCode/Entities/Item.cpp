//=====================================================
// Item.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "Item.hpp"
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "../Inventory.hpp"
#include <assert.h>

Items Item::s_itemsOnMap;

///=====================================================
/// 
///=====================================================
Item::Item(const XMLNode& itemNode, OpenGLRenderer* renderer) :
Entity(itemNode, renderer),
m_type(Undefined),
m_equippedSlot(Unequippable),
m_damageReductionPercent(0.0f),
m_numUses(1),
m_healing(0),
m_damage(0, 0){
	m_name = GetStringAttribute(itemNode, "name");

	std::string itemType = GetStringAttribute(itemNode, "type");

	m_damageReductionPercent = GetFloatAttribute(itemNode, "damageReductionPercent", m_damageReductionPercent);
	m_numUses = GetIntAttribute(itemNode, "numUses", m_numUses);
	m_healing = GetIntAttribute(itemNode, "healing", m_healing);
	m_damage = IntVec2(GetIntAttribute(itemNode, "minDamage", m_damage.x), GetIntAttribute(itemNode, "maxDamage", m_damage.y));

	RGBAchars defaultColor = RGBAchars::WHITE;
	char defaultGlyph = ':';

	if (itemType == "Weapon"){
		m_type = ItemType::Weapon;
	}
	else if (itemType == "Armor"){
		m_type = ItemType::Armor;
	}
	else if (itemType == "Consumable"){
		m_type = ItemType::Consumable;
	}
	else{ //unknown item
		assert(false);
	}

	if (m_type != ItemType::Undefined){
		defaultColor = ITEM_COLOR_TABLE[m_type];
		defaultGlyph = ITEM_SYMBOL_TABLE[m_type];
	}


	std::string itemEquipSlot = GetStringAttribute(itemNode, "equip");

	if (itemEquipSlot == "Weapon"){
		m_equippedSlot = EquipmentSlot::WeaponSlot;
	}
	else if (itemEquipSlot == "Chest"){
		m_equippedSlot = EquipmentSlot::Chest;
	}
	else if (itemEquipSlot == "Head"){
		m_equippedSlot = EquipmentSlot::Head;
	}
	else if (itemEquipSlot == "Ring"){
		m_equippedSlot = EquipmentSlot::Ring;
	}
	else if (itemEquipSlot == "Hands"){
		m_equippedSlot = EquipmentSlot::Hands;
	}
	else if (itemEquipSlot == "Feet"){
		m_equippedSlot = EquipmentSlot::Feet;
	}
	else if (itemEquipSlot == "Legs"){
		m_equippedSlot = EquipmentSlot::Legs;
	}

	m_glyph = GetCharAttribute(itemNode, "glyph", defaultGlyph);
	m_color = GetColorAttribute(itemNode, "color", defaultColor);

	m_textRenderer.SetInputString(std::string(1, m_glyph));
	m_textRenderer.SetInputStringColor(m_color);
}

///=====================================================
/// 
///=====================================================
Item::Item(const Item& other, OpenGLRenderer* renderer, const XMLNode& saveData /*= XMLNode::emptyNode()*/):
Entity(other, renderer, saveData),
m_type(other.m_type),
m_equippedSlot(other.m_equippedSlot),
m_damageReductionPercent(other.m_damageReductionPercent),
m_numUses(other.m_numUses),
m_healing(other.m_healing),
m_damage(other.m_damage){
	if (!saveData.isEmpty()){
		m_glyph = GetCharAttribute(saveData, "glyph", m_glyph);
		m_color = GetColorAttribute(saveData, "color", m_color);
	}

	m_textRenderer.SetInputString(std::string(1, m_glyph));
	m_textRenderer.SetInputStringColor(m_color);
}

///=====================================================
/// 
///=====================================================
void Item::AddToMap(Map* map, const IntVec2& position){
	assert(map != nullptr);

	m_position = position;
	m_map = map;

	Tile& tile = map->GetTileAtPosition(position);

	Inventory*& tileItems = tile.m_items;
	if (tileItems == nullptr){
		tileItems = new Inventory();
	}

	tileItems->AddToBackpack(*this);
	s_itemsOnMap.push_back(this);
}

///=====================================================
/// 
///=====================================================
void Item::Render(OpenGLRenderer* renderer, bool forceVisible /*= false*/){
	if (m_position.x == -1) return;

	const Tile& tile = m_map->GetTileAtPosition(m_position);
	if (tile.m_actor != nullptr || tile.m_feature != nullptr) return;

	if (tile.m_items == nullptr || tile.m_items->m_backpack.empty()){
		assert(false);
		return;
	}

	if (tile.m_items->m_backpack.size() > 1){ //multiple items located in the same location
		m_textRenderer.SetInputString("*");
	}
	
	Entity::Render(renderer, forceVisible);

	if (tile.m_items->m_backpack.size() > 1){
		m_textRenderer.SetInputString(std::string(1, m_glyph));
	}
}

///=====================================================
/// 
///=====================================================
void Item::SaveGame(XMLNode& entitiesNode) const{
	XMLNode itemNode = entitiesNode.addChild("Item");
	Entity::SaveGame(itemNode);

	SetCharAttribute(itemNode, "glyph", m_glyph, ITEM_SYMBOL_TABLE[m_type]);
	SetColorAttribute(itemNode, "color", m_color, ITEM_COLOR_TABLE[m_type]);
}
