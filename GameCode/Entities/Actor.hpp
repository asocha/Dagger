//=====================================================
// Actor.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Actor__
#define __included_Actor__

#include "Entity.hpp"
#include "../Inventory.hpp"
#include "../Pathfinding.hpp"
#include <set>
class Tile;
class Faction;


struct RememberedActor;
typedef std::multimap<float, class Actor*> Actors;

class Actor : public Entity{
public:
	Path m_path;
	RememberedActor* m_targetEnemy;
	RememberedActor* m_targetAlly;
	std::vector<Actor*> m_visibleActors;
	std::map<int, RememberedActor> m_previouslyVisibleActors;
	std::set<Tile*> m_visibleTiles;
	Faction* m_faction;
	Inventory* m_inventory;
	float m_chanceToHit;
	bool m_isInvulnerable;
	int m_invulnerabilityExpirationTurn;
	bool m_canUseFeatures;

	static Actors s_actorsOnMap;

	Actor(OpenGLRenderer* renderer);
	Actor(const XMLNode& actorNode, OpenGLRenderer* renderer);
	Actor(const Actor& other, OpenGLRenderer* renderer, const XMLNode& saveData = XMLNode::emptyNode());
	virtual ~Actor();

	inline bool IsActor() const { return true; }

	void AddToMap(Map* map, const IntVec2& position);
	
	virtual bool Think(bool isMainThink) = 0;

	bool CanMoveOneStep(const IntVec2& direction) const;
	void MoveOneStep(const IntVec2& direction);

	bool CanMoveToLocation(const IntVec2& location) const;
	void MoveToLocation(const IntVec2& direction);

	void PickUpItem();
	void PathfindToDestination(const IntVec2& goal, bool doActorsBlock, bool calcFullPath = true);
	bool CanSeePlayer() const;

	virtual void Render(OpenGLRenderer* renderer, bool forceVisible = false);

	void Die();
	void ReactToDeadActor(const Actor& actor);

	int GetFactionStatus(unsigned int factionID, int entityID) const;
	void AdjustFactionStatus(unsigned int factionID, int adjustment, int entityID);

	virtual void SaveGame(XMLNode& actorNode) const;
	void ResolvePointersFromLoadGame();
};

struct RememberedActor{
	bool m_isPlayer;
	int m_ID;
	unsigned int m_factionID;
	IntVec2 m_position;
	int m_turn;
	bool m_canAOEBuff;
	bool m_isFromYellForHelp;
	std::string m_name;

	RememberedActor();
	RememberedActor(const Actor& actor);

	static Actor* GetActorFromRememberedActor(const RememberedActor& rememberedActor);
};

#endif