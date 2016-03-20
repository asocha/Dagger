//=====================================================
// Player.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Player__
#define __included_Player__

#include "Actor.hpp"
#include "Engine/Input/InputSystem.hpp"

class Player : public Actor{
private:
	void UpdateMovement();
	void MoveOrAttackInDirection(const IntVec2& direction);
	void UpdateMapVisibility();

public:
	int m_killCount;

	Player(OpenGLRenderer* renderer);
	Player(const Player& other, OpenGLRenderer* renderer, const XMLNode& playerNode);
	inline ~Player(){}

	bool Think(bool isMainThink);
	inline bool IsReadyToThink() const{ return (s_theInputSystem->GetAnyKeyWentDown() && s_theInputSystem->GetCurrentCharacterDown() != -1 && s_theInputSystem->GetCurrentCharacterDown() != ' ' && s_theInputSystem->GetCurrentCharacterDown() != '`'); }

	void AddToMap(Map* map, const IntVec2& position);

	inline bool IsPlayer() const { return true; }

	void SaveGame(XMLNode& entitiesNode) const;
};

#endif