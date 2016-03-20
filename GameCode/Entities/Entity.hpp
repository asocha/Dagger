//=====================================================
// Entity.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Entity__
#define __included_Entity__

#include "Engine/Math/IntVec2.hpp"
class Map;
#include "Engine/Console/Console.hpp"
#include "Engine/XML/xmlParser.h"
class Actor;


class Entity{
private:
	static int s_nextID;
	
protected:
	Console m_textRenderer;
	RGBAchars m_color;

public:
	static int s_turnCount;

	std::string m_name;
	char m_glyph;
	Map* m_map;
	IntVec2 m_position;
	int m_health;
	int m_maxHealth;
	bool m_isDead;
	int m_ID;

	static std::map<int, Entity*> s_loadGameLinkMap;

	Entity(OpenGLRenderer* renderer);
	Entity(const XMLNode& entityNode, OpenGLRenderer* renderer);
	Entity(const Entity& other, OpenGLRenderer* renderer, const XMLNode& entityNode = XMLNode::emptyNode());
	inline Entity(){}
	virtual inline ~Entity(){}
	
	virtual void AddToMap(Map* map, const IntVec2& position);

	virtual void Render(OpenGLRenderer* renderer, bool forceVisible = false);

	inline virtual bool IsPlayer() const { return false; }
	inline virtual bool IsActor() const { return false; }
	virtual void TakeDamage(int damage, const Actor* attacker);
	virtual void Die();

	void ShutdownRendering(const OpenGLRenderer* renderer);

	virtual void SaveGame(XMLNode& entityNode) const;
	virtual void ResolvePointersFromLoadGame();
};

#endif