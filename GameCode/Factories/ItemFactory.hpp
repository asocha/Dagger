//=====================================================
// ItemFactory.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_ItemFactory__
#define __included_ItemFactory__

class Item;
#include "Engine/XML/xmlParser.h"
class OpenGLRenderer;

#include <map>

typedef std::map<unsigned int, class ItemFactory*> ItemFactoryRegistry;

class ItemFactory{
private:
	const Item* m_itemBlueprint;
	const OpenGLRenderer* m_theRenderer;

public:
	ItemFactory(const XMLNode& itemNode, OpenGLRenderer* renderer);
	~ItemFactory();

	static ItemFactoryRegistry s_factories;

	Item* SpawnItem(const XMLNode& saveData = XMLNode::emptyNode()) const;

	static bool LoadAllItemFactories(OpenGLRenderer* renderer);
	static void DeleteItemFactories();
	static ItemFactory* FindFactoryByName(const std::string& name);
};

#endif