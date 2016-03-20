//=====================================================
// NPCFactory.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_NPCFactory__
#define __included_NPCFactory__

class NPC;
#include "Engine/XML/xmlParser.h"
class OpenGLRenderer;

#include <map>

typedef std::map<unsigned int, class NPCFactory*> NPCFactoryRegistry;

class NPCFactory{
private:
	const NPC* m_npcBlueprint;
	const OpenGLRenderer* m_theRenderer;

public:
	NPCFactory(const XMLNode& npcNode, OpenGLRenderer* renderer);
	~NPCFactory();

	static NPCFactoryRegistry s_factories;
	
	NPC* SpawnNPC(const XMLNode& saveData = XMLNode::emptyNode()) const;

	static bool LoadAllNPCFactories(OpenGLRenderer* renderer);
	static void DeleteNPCFactories();
	static NPCFactory* FindFactoryByName(const std::string& name);
};

#endif