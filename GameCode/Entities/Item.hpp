//=====================================================
// Item.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Item__
#define __included_Item__

#include "Entity.hpp"

typedef std::vector<class Item*> Items;

class Item : public Entity{
public:
	enum ItemType{
		Undefined = -1,
		Weapon,
		Armor,
		Consumable,
		NumItemTypes
	};

	enum EquipmentSlot{
		Unequippable = -1,
		Head,
		Chest,
		WeaponSlot,
		Ring,
		Feet,
		Hands,
		Legs,
		NumEquipmentSlots
	};

private:
	int m_numUses; //not yet used

public:
	ItemType m_type;
	EquipmentSlot m_equippedSlot;

	int m_healing;
	IntVec2 m_damage;
	float m_damageReductionPercent;

	static Items s_itemsOnMap;

	Item(const XMLNode& itemNode, OpenGLRenderer* renderer);
	Item(const Item& other, OpenGLRenderer* renderer, const XMLNode& saveData = XMLNode::emptyNode());
	
	void AddToMap(Map* map, const IntVec2& position);
	void Render(OpenGLRenderer* renderer, bool forceVisible = false);

	void SaveGame(XMLNode& entitiesNode) const;
};

const char ITEM_SYMBOL_TABLE[Item::NumItemTypes]{'/', ']', '~'};
const RGBAchars ITEM_COLOR_TABLE[Item::NumItemTypes]{RGBAchars(255, 255, 255, 255), RGBAchars(255, 255, 255, 255), RGBAchars(255, 255, 255, 255)};

#endif