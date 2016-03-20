//=====================================================
// Faction.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "Faction.hpp"
#include "Engine/Core/StringTable.hpp"
#include <assert.h>
#include "Engine/XML/XMLParsingSupport.hpp"
#include "Entities/Entity.hpp"

FactionMap Faction::s_factionMap;

///=====================================================
/// 
///=====================================================
Faction::Faction(const XMLNode& factionNode) :
m_factionRelationshipMap(),
m_entityRelationshipMap(),
m_factionID(0){
	m_factionID = GetStringID(GetStringAttribute(factionNode, "name"));

	for (int i = 0; i < factionNode.nChildNode(); ++i){
		XMLNode relationShipNode = factionNode.getChildNode(i);
		std::string factionStatusString = GetStringAttribute(relationShipNode, "status", "Neutral");

		FactionStatus factionStatus;
		if (factionStatusString == "Hated"){
			factionStatus = FactionStatus::Hated;
		}
		else if (factionStatusString == "Unfriendly"){
			factionStatus = FactionStatus::Unfriendly;
		}
		else if (factionStatusString == "Friendly"){
			factionStatus = FactionStatus::Friendly;
		}
		else if (factionStatusString == "Loved"){
			factionStatus = FactionStatus::Loved;
		}
		else{ //Neutral
			factionStatus = FactionStatus::Neutral;
		}

		m_factionRelationshipMap.emplace(GetStringID(GetStringAttribute(relationShipNode, "name")), factionStatus);
	}
}

///=====================================================
/// 
///=====================================================
Faction::Faction(const Faction& other, const XMLNode& relationshipsNode /*= XMLNode::emptyNode()*/):
m_factionRelationshipMap(other.m_factionRelationshipMap),
m_entityRelationshipMap(other.m_entityRelationshipMap),
m_factionID(other.m_factionID){
	if (!relationshipsNode.isEmpty()){ //load saved relationships
		XMLNode factionRelationshipsNode = relationshipsNode.getChildNode("Factions");
		if (!factionRelationshipsNode.isEmpty()){
			for (int i = 0; i < factionRelationshipsNode.nChildNode(); ++i){
				XMLNode relationshipNode = factionRelationshipsNode.getChildNode(i);
				int factionID = GetIntAttribute(relationshipNode, "factionID");
				FactionStatus factionStatus = (FactionStatus)GetIntAttribute(relationshipNode, "factionStatus");
				m_factionRelationshipMap[factionID] = factionStatus;
			}
		}

		XMLNode entityRelationshipsNode = relationshipsNode.getChildNode("Entities");
		if (!entityRelationshipsNode.isEmpty()){
			for (int i = 0; i < entityRelationshipsNode.nChildNode(); ++i){
				XMLNode relationshipNode = entityRelationshipsNode.getChildNode(i);
				int entityID = GetIntAttribute(relationshipNode, "entityID");
				FactionStatus factionStatus = (FactionStatus)GetIntAttribute(relationshipNode, "factionStatus");
				m_entityRelationshipMap[entityID] = factionStatus;
			}
		}
	}
}

///=====================================================
/// 
///=====================================================
void Faction::LoadFactions(){
	XMLNode factionsNode = XMLNode::parseFile("Data/factions.xml", "Factions");
	XMLNode factionNode = factionsNode.getChildNode(0);
	for (int i = 1; !factionNode.isEmpty(); ++i){
		std::string factionName = GetStringAttribute(factionNode, "name");
		Faction* newFaction = new Faction(factionNode);
		s_factionMap.emplace(GetStringID(factionName), newFaction);

		factionNode = factionsNode.getChildNode(i);
	}
}

///=====================================================
/// 
///=====================================================
void Faction::DeleteFactions(){
	for (FactionMap::const_iterator factionIter = s_factionMap.cbegin(); factionIter != s_factionMap.cend(); ++factionIter){
		delete factionIter->second;
	}
}

///=====================================================
/// 
///=====================================================
Faction* Faction::GetNewFactionByName(const std::string& name, const XMLNode& relationshipsNode /*= XMLNode::emptyNode()*/){
	FactionMap::const_iterator factionIter = s_factionMap.find(GetStringID(name));
	if (factionIter == s_factionMap.cend()){
		assert(false);
		return nullptr;
	}
	return new Faction(*(factionIter->second), relationshipsNode);
}

///=====================================================
/// 
///=====================================================
void Faction::SaveGame(XMLNode& relationshipsNode) const{
	if (!m_factionRelationshipMap.empty()){
		XMLNode factionRelationshipsNode = relationshipsNode.addChild("Factions");
		for (FactionRelationshipMap::const_iterator factionRelationshipIter = m_factionRelationshipMap.cbegin(); factionRelationshipIter != m_factionRelationshipMap.cend(); ++factionRelationshipIter){
			if (factionRelationshipIter->second == Neutral) continue;

			XMLNode relationshipNode = factionRelationshipsNode.addChild("Relationship");
			SetIntAttribute(relationshipNode, "factionID", factionRelationshipIter->first);
			SetIntAttribute(relationshipNode, "factionStatus", factionRelationshipIter->second);
		}
	}
	if (!m_entityRelationshipMap.empty()){
		XMLNode entityRelationshipsNode = relationshipsNode.addChild("Entities");
		for (EntityRelationshipMap::const_iterator entityRelationshipIter = m_entityRelationshipMap.cbegin(); entityRelationshipIter != m_entityRelationshipMap.cend(); ++entityRelationshipIter){
			if (entityRelationshipIter->second == Neutral) continue;
			
			XMLNode relationshipNode = entityRelationshipsNode.addChild("Relationship");
			SetIntAttribute(relationshipNode, "entityID", entityRelationshipIter->first);
			SetIntAttribute(relationshipNode, "factionStatus", entityRelationshipIter->second);
		}
	}
}

///=====================================================
/// 
///=====================================================
void Faction::ResolvePointersFromLoadGame(){
	for (EntityRelationshipMap::const_iterator entityRelationshipIter = m_entityRelationshipMap.cbegin(); entityRelationshipIter != m_entityRelationshipMap.cend();){
		std::pair<int, FactionStatus> relationship = *entityRelationshipIter;
		entityRelationshipIter = m_entityRelationshipMap.erase(entityRelationshipIter);

		std::map<int, Entity*>::const_iterator loadGameIter = Entity::s_loadGameLinkMap.find(relationship.first);
		if (loadGameIter != Entity::s_loadGameLinkMap.cend()){ //otherwise, the related entity wasn't saved (hopefully because it had died)
			relationship.first = loadGameIter->second->m_ID;
			m_entityRelationshipMap.insert(relationship);
		}
	}
}
