//=====================================================
// Tile.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Tile__
#define __included_Tile__

class Actor;
class Feature;
class Inventory;
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2D.hpp"
class OpenGLRenderer;
class Console;

enum TileType{
	OutsideOfWorld,
	Wall,
	Air,
	NumTiles
};

class Tile{
public:
	TileType m_tileType;
	IntVec2 m_position;
	Actor* m_actor;
	Inventory* m_items;
	Feature* m_feature;
	bool m_isCompletelyHidden;
	bool m_isVisibleToPlayer;
	bool m_hasBeenSeen;

	static std::string TILE_SYMBOL_TABLE[NumTiles];

	Tile(const IntVec2& position, TileType type = TileType::Air);
	~Tile();

	void SetTileType(TileType newType);
	inline TileType GetTileType() const{ return m_tileType; }
	static TileType ConvertCharToTileType(char character);

	bool CanBeMovedInto() const;
	
	void Render(Console& textRenderer, const OpenGLRenderer* renderer, float fontSize, const Vec2& offset, bool forceVisible = false);
};

#endif