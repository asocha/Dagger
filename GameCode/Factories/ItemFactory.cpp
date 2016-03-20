//=====================================================
// ItemFactory.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "ItemFactory.hpp"
#include "../Entities/Item.hpp"
#include "Engine/Core/Utilities.hpp"
#include "Engine/Core/StringTable.hpp"

ItemFactoryRegistry ItemFactory::s_factories;

///=====================================================
/// 
///=====================================================
ItemFactory::ItemFactory(const XMLNode& itemNode, OpenGLRenderer* renderer) :
m_itemBlueprint(nullptr),
m_theRenderer(renderer){
	m_itemBlueprint = new Item(itemNode, renderer);

	//pull out ranges here
}

///=====================================================
/// 
///=====================================================
ItemFactory::~ItemFactory(){
	delete m_itemBlueprint;
}

///=====================================================
/// 
///=====================================================
Item* ItemFactory::SpawnItem(const XMLNode& saveData /*= XMLNode::emptyNode()*/) const{
	return new Item(*m_itemBlueprint, (OpenGLRenderer*)m_theRenderer, saveData);
}

///=====================================================
/// 
///=====================================================
bool ItemFactory::LoadAllItemFactories(OpenGLRenderer* renderer){
	std::vector<std::string> factories;
	bool foundFiles = FindAllFilesOfType("Data/Items/", "*.item.xml", factories);
	RECOVERABLE_ASSERT(foundFiles == true);

	for (std::vector<std::string>::const_iterator factoryIter = factories.cbegin(); factoryIter != factories.cend(); ++factoryIter){
		XMLNode itemsNode = XMLNode::parseFile(factoryIter->c_str());
		XMLNode itemNode = itemsNode.getChildNode(0);

		int childNum = 1;

		while (!itemNode.isEmpty()){
			ItemFactory* newFactory = new ItemFactory(itemNode, renderer);

			s_factories.emplace(GetStringID(newFactory->m_itemBlueprint->m_name), newFactory);

			itemNode = itemsNode.getChildNode(childNum++);
		}
	}

	return foundFiles;
}

///=====================================================
/// 
///=====================================================
void ItemFactory::DeleteItemFactories(){
	for (ItemFactoryRegistry::const_iterator factoryIter = ItemFactory::s_factories.cbegin(); factoryIter != ItemFactory::s_factories.cend(); ++factoryIter){
		delete factoryIter->second;
	}
}

///=====================================================
/// 
///=====================================================
ItemFactory* ItemFactory::FindFactoryByName(const std::string& name){
	ItemFactoryRegistry::iterator factoryIter = s_factories.find(GetStringID(name));
	if (factoryIter == s_factories.end())
		return nullptr;
	return factoryIter->second;
}
