//=====================================================
// Map.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Map__
#define __included_Map__

#include <vector>
#include "Tile.hpp"
#include "Engine/Math/IntVec2.hpp"
class OpenGLRenderer;
struct XMLNode;
#include "Engine/Console/Console.hpp"

typedef std::vector<Tile> Tiles;

class Map{
private:
	int m_seed;
	Tile* m_edgeOfWorld;
	Console m_textRenderer;

public:
	Tiles m_tiles;
	IntVec2 m_size;
	float m_fontSize;
	Vec2 m_renderOffset;

	Map(int size);
	Map(const IntVec2& size);
	Map(const XMLNode& saveGameNode);

	void DestroyMap(const OpenGLRenderer* renderer);
	void CreateMap(OpenGLRenderer* renderer);
	IntVec2 CreateMap(const XMLNode& saveGameNode, OpenGLRenderer* renderer);

	void Render(const OpenGLRenderer* renderer, bool forceAllTilesVisible = false);

	Tile& GetTileAtPosition(const IntVec2& position);
	Tile& GetRandomTile();
	std::vector<Tile*>& GetTilesWithinRadius(const IntVec2& position, int radius = 1);
	std::vector<Tile*>& GetTilesWithinRadius(const IntVec2& position, TileType type, int radius = 1, bool onlyCanBeMovedInto = false);
	std::vector<Tile*>& GetVisibleTilesWithinRadius(const Actor& actor, const IntVec2& position, TileType type, int radius = 1, bool onlyCanBeMovedInto = false);
	std::vector<Tile*>& GetAllTilesOfType(TileType type, bool onlyCanBeMovedInto = false) const;

	void SaveGame(XMLNode& mapNode, XMLNode& mapVisibilityNode) const;
};

#endif