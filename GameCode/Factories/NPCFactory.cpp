//=====================================================
// NPCFactory.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "NPCFactory.hpp"
#include "../Entities/NPC.hpp"
#include "Engine/Core/Utilities.hpp"
#include "Engine/Core/StringTable.hpp"

NPCFactoryRegistry NPCFactory::s_factories;

///=====================================================
/// 
///=====================================================
NPCFactory::NPCFactory(const XMLNode& npcNode, OpenGLRenderer* renderer) :
m_npcBlueprint(nullptr),
m_theRenderer(renderer){
	m_npcBlueprint = new NPC(npcNode, renderer);

	//pull out ranges here
}

///=====================================================
/// 
///=====================================================
NPCFactory::~NPCFactory(){
	delete m_npcBlueprint;
}

///=====================================================
/// 
///=====================================================
NPC* NPCFactory::SpawnNPC(const XMLNode& saveData/* = XMLNode::emptyNode()*/) const{
	return new NPC(*m_npcBlueprint, (OpenGLRenderer*)m_theRenderer, saveData);
}

///=====================================================
/// 
///=====================================================
bool NPCFactory::LoadAllNPCFactories(OpenGLRenderer* renderer){
	std::vector<std::string> factories;
	bool foundFiles = FindAllFilesOfType("Data/NPCs/", "*.npc.xml", factories);
	RECOVERABLE_ASSERT(foundFiles == true);

	for (std::vector<std::string>::const_iterator factoryIter = factories.cbegin(); factoryIter != factories.cend(); ++factoryIter){
		XMLNode npcNode = XMLNode::parseFile(factoryIter->c_str(), "NPC");

		NPCFactory* newFactory = new NPCFactory(npcNode, renderer);

		s_factories.emplace(GetStringID(newFactory->m_npcBlueprint->m_name), newFactory);
	}

	return foundFiles;
}

///=====================================================
/// 
///=====================================================
void NPCFactory::DeleteNPCFactories(){
	for (NPCFactoryRegistry::const_iterator factoryIter = NPCFactory::s_factories.cbegin(); factoryIter != NPCFactory::s_factories.cend(); ++factoryIter){
		delete factoryIter->second;
	}
}

///=====================================================
/// 
///=====================================================
NPCFactory* NPCFactory::FindFactoryByName(const std::string& name){
	NPCFactoryRegistry::iterator factoryIter = s_factories.find(GetStringID(name));
	if (factoryIter == s_factories.end())
		return nullptr;
	return factoryIter->second;
}
