//=====================================================
// Inventory.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Inventory__
#define __included_Inventory__

#include <map>
#include "Entities/Item.hpp"

typedef std::multimap<Item::ItemType, Item*> Backpack;
typedef std::map<Item::EquipmentSlot, Item*> Equipment;

class Inventory{
public:
	Backpack m_backpack;
	Equipment m_equipment;

	Inventory();
	Inventory(const XMLNode& inventoryNode);
	~Inventory();
	
	void AddToBackpack(const Item& item);
	void RemoveFromBackpack(const Item& item);
	Item* RemoveFirstItemFromBackpack();
	void AddToEquipment(const Item& item);

	void SaveGame(XMLNode& inventoryNode) const;
};

#endif