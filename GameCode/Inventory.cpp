//=====================================================
// Inventory.cpp
// by Andrew Socha
//=====================================================

#include "Inventory.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "Factories/ItemFactory.hpp"

///=====================================================
/// 
///=====================================================
Inventory::Inventory():
m_backpack(),
m_equipment(){
}

///=====================================================
/// 
///=====================================================
Inventory::Inventory(const XMLNode& inventoryNode):
m_backpack(),
m_equipment(){
	XMLNode backpackNode = inventoryNode.getChildNode("Backpack");
	for (int i = 0; i < backpackNode.nChildNode(); ++i){
		XMLNode itemNode = backpackNode.getChildNode(i);
		std::string itemName = GetStringAttribute(itemNode, "name");
		ItemFactory* factory = ItemFactory::FindFactoryByName(itemName);

		Item* item = factory->SpawnItem(itemNode);

		m_backpack.emplace(item->m_type, item);
	}

	XMLNode equipmentNode = inventoryNode.getChildNode("Equipment");
	for (int i = 0; i < equipmentNode.nChildNode(); ++i){
		XMLNode itemNode = equipmentNode.getChildNode(i);
		std::string itemName = GetStringAttribute(itemNode, "name");
		ItemFactory* factory = ItemFactory::FindFactoryByName(itemName);

		Item* item = factory->SpawnItem(itemNode);

		m_equipment.emplace(item->m_equippedSlot, item);
	}
	
}

///=====================================================
/// 
///=====================================================
Inventory::~Inventory(){
	for (Backpack::const_iterator backpackIter = m_backpack.cbegin(); backpackIter != m_backpack.cend(); ++backpackIter){
		delete backpackIter->second;
	}
	for (Equipment::const_iterator equipmentIter = m_equipment.cbegin(); equipmentIter != m_equipment.cend(); ++equipmentIter){
		delete equipmentIter->second;
	}
}

///=====================================================
/// 
///=====================================================
void Inventory::AddToBackpack(const Item& item){
	m_backpack.emplace(item.m_type, (Item*)&item);
}

///=====================================================
/// 
///=====================================================
void Inventory::RemoveFromBackpack(const Item& item){
	for (Backpack::iterator backpackIter = m_backpack.find(item.m_type); backpackIter != m_backpack.end() && backpackIter->first == item.m_type; ++backpackIter){
		if (backpackIter->second == &item){
			m_backpack.erase(backpackIter);
			return;
		}
	}

	RECOVERABLE_ERROR(); //item isn't in backpack
}

///=====================================================
/// 
///=====================================================
Item* Inventory::RemoveFirstItemFromBackpack(){
	FATAL_ASSERT(!m_backpack.empty());

	Item* item = m_backpack.begin()->second;
	m_backpack.erase(m_backpack.begin());

	return item;
}

///=====================================================
/// 
///=====================================================
void Inventory::AddToEquipment(const Item& item){
	Equipment::iterator equipmentIter = m_equipment.find(item.m_equippedSlot);
	if (equipmentIter == m_equipment.end()){
		m_equipment.emplace(item.m_equippedSlot, (Item*)&item);
	}
	else{ //already equipped, move worse item to backpack
		if (item.m_damage.x + item.m_damage.y > equipmentIter->second->m_damage.x + equipmentIter->second->m_damage.y){
			m_backpack.emplace(equipmentIter->second->m_type, equipmentIter->second);
			m_equipment.erase(equipmentIter);
			m_equipment.emplace(item.m_equippedSlot, (Item*)&item);
		}
		else{
			m_backpack.emplace(item.m_type, (Item*)&item);
		}
	}
}

///=====================================================
/// 
///=====================================================
void Inventory::SaveGame(XMLNode& inventoryNode) const{
	if (!m_backpack.empty()){
		XMLNode backpackNode = inventoryNode.addChild("Backpack");
		for (Backpack::const_iterator backpackIter = m_backpack.cbegin(); backpackIter != m_backpack.cend(); ++backpackIter){
			backpackIter->second->SaveGame(backpackNode);
		}
	}

	if (!m_equipment.empty()){
		XMLNode equipmentNode = inventoryNode.addChild("Equipment");
		for (Equipment::const_iterator equipmentIter = m_equipment.cbegin(); equipmentIter != m_equipment.cend(); ++equipmentIter){
			equipmentIter->second->SaveGame(equipmentNode);
		}
	}
}
