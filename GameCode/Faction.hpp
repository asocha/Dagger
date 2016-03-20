//=====================================================
// Faction.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Faction__
#define __included_Faction__

#include "Engine/XML/xmlParser.h"
#include <map>

enum FactionStatus{
	Hated = -50,
	Unfriendly = -10,
	Neutral = 0,
	Friendly = 10,
	Loved = 50
};

typedef std::map<unsigned int, class Faction*> FactionMap;
typedef std::map<unsigned int, FactionStatus> FactionRelationshipMap;
typedef std::map<int, FactionStatus> EntityRelationshipMap;

class Faction{
private:
	static FactionMap s_factionMap;

public:
	unsigned int m_factionID;
	FactionRelationshipMap m_factionRelationshipMap;
	EntityRelationshipMap m_entityRelationshipMap;

	Faction(const XMLNode& factionNode);
	Faction(const Faction& other, const XMLNode& relationshipsNode = XMLNode::emptyNode());

	static void LoadFactions();
	static void DeleteFactions();
	static Faction* GetNewFactionByName(const std::string& name, const XMLNode& relationshipsNode = XMLNode::emptyNode());

	void SaveGame(XMLNode& relationshipsNode) const;
	void ResolvePointersFromLoadGame();
};

#endif