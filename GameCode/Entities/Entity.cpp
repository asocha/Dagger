//=====================================================
// Entity.cpp
// by Andrew Socha
//=====================================================

#include "Entity.hpp"
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "Actor.hpp"
#include "NPC.hpp"
#include "../AIBehaviors/BaseAIBehavior.hpp"

int Entity::s_nextID = 0;
int Entity::s_turnCount = 0;

std::map<int, Entity*> Entity::s_loadGameLinkMap;

///=====================================================
/// 
///=====================================================
Entity::Entity(OpenGLRenderer* renderer) :
m_health(0),
m_maxHealth(0),
m_position(-1, -1),
m_name(""),
m_ID(s_nextID++),
m_glyph('!'),
m_map(nullptr),
m_textRenderer(),
m_isDead(false),
m_color(RGBAchars::WHITE){
	m_textRenderer.Startup(renderer);
	m_textRenderer.ToggleCursor();
}

///=====================================================
/// 
///=====================================================
Entity::Entity(const XMLNode& entityNode, OpenGLRenderer* renderer) :
m_health(0),
m_maxHealth(0),
m_position(-1, -1),
m_name(""),
m_ID(s_nextID++),
m_glyph('!'),
m_map(nullptr),
m_textRenderer(),
m_isDead(false),
m_color(RGBAchars::WHITE){
	m_name = GetStringAttribute(entityNode, "name", m_name);

	m_health = GetIntAttribute(entityNode, "health", m_health);
	m_maxHealth = m_health;

	m_textRenderer.Startup(renderer);
	m_textRenderer.ToggleCursor();
}

///=====================================================
/// 
///=====================================================
Entity::Entity(const Entity& other, OpenGLRenderer* renderer, const XMLNode& entityNode /*= XMLNode::emptyNode()*/):
m_health(other.m_health),
m_maxHealth(other.m_maxHealth),
m_position(other.m_position),
m_name(other.m_name),
m_ID(s_nextID++),
m_glyph(other.m_glyph),
m_map(nullptr),
m_textRenderer(),
m_isDead(other.m_isDead),
m_color(other.m_color){
	if (!entityNode.isEmpty()){
		m_name = GetStringAttribute(entityNode, "name", m_name);

		int linkID = GetIntAttribute(entityNode, "linkID");
		s_loadGameLinkMap[linkID] = this;

		m_position = GetIntVec2Attribute(entityNode, "position", m_position);
		m_health = GetIntAttribute(entityNode, "health", m_health);
		m_maxHealth = GetIntAttribute(entityNode, "maxHealth", m_maxHealth);
	}

	m_textRenderer.Startup(renderer);
	m_textRenderer.ToggleCursor();
}

///=====================================================
/// 
///=====================================================
void Entity::AddToMap(Map* /*map*/, const IntVec2& /*position*/){
	//do nothing... subclasses should implement this if they want to be add-able
}

///=====================================================
/// 
///=====================================================
void Entity::Render(OpenGLRenderer* renderer, bool forceVisible /*= false*/){
	if (m_isDead) return;
	const Tile& tile = m_map->GetTileAtPosition(m_position);
	if (!tile.m_isVisibleToPlayer && !forceVisible) return;

	if (IsActor() && ((Actor*)this)->m_isInvulnerable){
		RGBAchars tempColor = m_textRenderer.GetInputStringColor();
		m_textRenderer.SetInputStringColor(RGBAchars::RED);
		m_textRenderer.RenderText(renderer, Font::CONSOLAS, m_map->m_fontSize, m_map->m_renderOffset + Vec2(103.0f + (float)m_position.x * (16.4f / 35.0f) * m_map->m_fontSize, 100.0f + (float)m_position.y * m_map->m_fontSize));
		m_textRenderer.SetInputStringColor(tempColor);
	}
	else{
		m_textRenderer.RenderText(renderer, Font::CONSOLAS, m_map->m_fontSize, m_map->m_renderOffset + Vec2(103.0f + (float)m_position.x * (16.4f / 35.0f) * m_map->m_fontSize, 100.0f + (float)m_position.y * m_map->m_fontSize));
	}
}

///=====================================================
/// 
///=====================================================
void Entity::TakeDamage(int damage, const Actor* /*attacker*/){
	m_health -= damage;
	if (m_health <= 0){
		Die();
	}
	else if (m_health > m_maxHealth){ //in case damage is negative (i.e. healing)
		m_health = m_maxHealth;
	}
}

///=====================================================
/// 
///=====================================================
void Entity::Die(){
	m_isDead = true;
	m_position = IntVec2(-1, -1);
}

///=====================================================
/// 
///=====================================================
void Entity::ShutdownRendering(const OpenGLRenderer* renderer){
	m_textRenderer.Shutdown(renderer);
}

///=====================================================
/// 
///=====================================================
void Entity::SaveGame(XMLNode& entityNode) const{
	SetStringAttribute(entityNode, "name", m_name);
	SetIntAttribute(entityNode, "linkID", m_ID);
	SetIntVec2Attribute(entityNode, "position", m_position);
	SetIntAttribute(entityNode, "health", m_health, m_maxHealth);
	SetIntAttribute(entityNode, "maxHealth", m_maxHealth, m_maxHealth); //won't actually ever save

	//don't save glyph or color, as child classes are responsible for knowing what their defaults are, so they will take care of it
}

///=====================================================
/// 
///=====================================================
void Entity::ResolvePointersFromLoadGame(){
	//do nothing
}
