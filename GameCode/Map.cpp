//=====================================================
// Map.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "Map.hpp"
#include "Engine/XML/xmlParser.h"
#include "Engine/Core/Utilities.hpp"
#include "Entities/Actor.hpp"
#include "Entities/Feature.hpp"

///=====================================================
/// 
///=====================================================
Map::Map(int size):
m_seed(0),
m_size((int)((float)size * 4.25f), size),
m_tiles(),
m_edgeOfWorld(nullptr),
m_fontSize(0.0f),
m_renderOffset(0.0f, 0.0f){
	m_fontSize = (700.0f / (float)m_size.y);
	m_renderOffset = Vec2(0.5f * (1395.0f - (m_fontSize * m_size.x * (15.0f / 32.0f))), 0.0f);
}

///=====================================================
/// 
///=====================================================
Map::Map(const IntVec2& size):
m_seed(0),
m_size(size),
m_tiles(),
m_edgeOfWorld(nullptr),
m_fontSize(0.0f),
m_renderOffset(0.0f, 0.0f){
	m_fontSize = (700.0f / (float)m_size.y);
	m_renderOffset = Vec2(0.5f * (1395.0f - (m_fontSize * m_size.x * (15.0f / 32.0f))), 0.0f);
}

///=====================================================
/// 
///=====================================================
Map::Map(const XMLNode& saveGameNode):
m_seed(0),
m_size(0, 0),
m_tiles(),
m_edgeOfWorld(nullptr),
m_fontSize(0.0f),
m_renderOffset(0.0f, 0.0f),
m_textRenderer(){
	const char* text = saveGameNode.getChildNode("Map").getText();
	if (text == nullptr){
		RECOVERABLE_ERROR();
		return;
	}

	std::vector<std::string> textVec = SplitString(text, '\n');
	m_size.y = textVec.size();
	m_size.x = (strlen(text) / m_size.y);
	if (textVec.at(0).at(textVec.at(0).length() - 1) == '\r')
		--m_size.x; //-1 to ignore '\r'

	if (m_size.x > m_size.y * 4.25f){
		m_fontSize = floor(700.0f / (floor((float)m_size.x * (1.0f / 4.25f)) + 1.0f)) + 1.0f;
		m_renderOffset = Vec2(0.0f, 0.5f * (700.0f - (m_fontSize * (float)m_size.y)));
	}
	else{
		m_fontSize = (700.0f / (float)m_size.y);
		m_renderOffset = Vec2(0.5f * (1395.0f - (m_fontSize * (float)m_size.x * (15.0f / 32.0f))), 0.0f);
	}
}

///=====================================================
/// 
///=====================================================
void Map::DestroyMap(const OpenGLRenderer* renderer){
	m_textRenderer.Shutdown(renderer);
	delete m_edgeOfWorld;

	for (std::vector<Tile>::iterator tileIter = m_tiles.begin(); tileIter != m_tiles.end(); ++tileIter){
		if (tileIter->m_actor != nullptr){
			delete tileIter->m_actor;
			tileIter->m_actor = nullptr;
		}
		delete tileIter->m_feature;
		tileIter->m_feature = nullptr;
		delete tileIter->m_items;
		tileIter->m_items = nullptr;
	}
}

///=====================================================
/// 
///=====================================================
void Map::CreateMap(OpenGLRenderer* renderer){
	m_textRenderer.Startup(renderer);
	m_textRenderer.ToggleCursor();

	m_tiles.reserve(m_size.x * m_size.y);

	for (int h = 0; h < m_size.y; ++h){
		for (int w = 0; w < m_size.x; ++w){
			m_tiles.emplace_back(IntVec2(w, h));
		}
	}
	m_edgeOfWorld = new Tile(IntVec2(-1, -1));
	m_edgeOfWorld->SetTileType(TileType::OutsideOfWorld);
}

///=====================================================
/// Returns player start location
///=====================================================
IntVec2 Map::CreateMap(const XMLNode& saveGameNode, OpenGLRenderer* renderer){
	m_textRenderer.Startup(renderer);
	m_textRenderer.ToggleCursor();

	m_edgeOfWorld = new Tile(IntVec2(-1, -1));
	m_edgeOfWorld->SetTileType(TileType::OutsideOfWorld);

	const char* mapGlyphs = saveGameNode.getChildNode("Map").getText();
	const char* mapVisibility = saveGameNode.getChildNode("Visibility").getText();
	
	IntVec2 playerStart(-1, -1);

	if (mapGlyphs == nullptr || mapVisibility == nullptr){
		RECOVERABLE_ERROR();
		return playerStart;
	}

	std::vector<std::string> mapGlyphsVec = SplitString(mapGlyphs, '\n');
	std::vector<std::string> mapVisibilityVec = SplitString(mapVisibility, '\n');

	m_tiles.reserve(m_size.x * m_size.y);

	int w = 0;
	int h = 0;
	for (int line = mapGlyphsVec.size() - 1; line >= 0; --line){
		for (unsigned int character = 0; character < mapGlyphsVec[line].size(); ++character){
			char nextChar = mapGlyphsVec[line][character];
			if (nextChar == '\r')
				continue;

			Tile newTile(IntVec2(w, h), Tile::ConvertCharToTileType(nextChar));
			if (mapVisibilityVec[line][character] != '?')
				newTile.m_hasBeenSeen = true;
			m_tiles.push_back(newTile);

			++w;
			if (w == m_size.x){
				w = 0;
				++h;
			}
		}
	}

	return playerStart;
}

///=====================================================
/// 
///=====================================================
void Map::Render(const OpenGLRenderer* renderer, bool forceAllTilesVisible /*= false*/){
	for (std::vector<Tile>::iterator tileIter = m_tiles.begin(); tileIter != m_tiles.end(); ++tileIter){
		tileIter->Render(m_textRenderer, renderer, m_fontSize, m_renderOffset, forceAllTilesVisible);
	}
}

///=====================================================
/// 
///=====================================================
Tile& Map::GetTileAtPosition(const IntVec2& position){
	if (position.x < 0 || position.y < 0 || position.x >= m_size.x || position.y >= m_size.y)
		return *m_edgeOfWorld;

	unsigned int tileIndex = position.x + position.y * m_size.x;
	return m_tiles.at(tileIndex);
}

///=====================================================
/// 
///=====================================================
Tile& Map::GetRandomTile(){
	int tileIndex = GetRandomIntLessThan(m_tiles.size());
	return m_tiles.at(tileIndex);
}

///=====================================================
/// 
///=====================================================
std::vector<Tile*>& Map::GetTilesWithinRadius(const IntVec2& position, int radius /*= 1*/){
	IntVec2 currentPos = position + IntVec2(-radius, -radius);
	static std::vector<Tile*> tiles;
	tiles.clear();

	for (int x = 0; x <= 2 * radius; ++x){
		for (int y = 0; y <= 2 * radius; ++y){
			tiles.push_back(&GetTileAtPosition(currentPos));
			++currentPos.y;
		}
		currentPos.y = position.y - radius;
		++currentPos.x;
	}

	return tiles;
}

///=====================================================
/// 
///=====================================================
std::vector<Tile*>& Map::GetTilesWithinRadius(const IntVec2& position, TileType type, int radius /*= 1*/, bool onlyCanBeMovedInto/* = false*/){
	IntVec2 currentPos = position + IntVec2(-radius, -radius);
	static std::vector<Tile*> tiles;
	tiles.clear();

	for (int x = 0; x <= 2 * radius; ++x){
		for (int y = 0; y <= 2 * radius; ++y){
			Tile& tile = GetTileAtPosition(currentPos);
			if (tile.GetTileType() == type && (!onlyCanBeMovedInto || tile.CanBeMovedInto()))
				tiles.push_back(&tile);
			++currentPos.y;
		}
		currentPos.y = position.y - radius;
		++currentPos.x;
	}

	return tiles;
}

///=====================================================
/// 
///=====================================================
std::vector<Tile*>& Map::GetVisibleTilesWithinRadius(const Actor& actor, const IntVec2& position, TileType type, int radius /*= 1*/, bool onlyCanBeMovedInto /*= false*/){
	IntVec2 currentPos = position + IntVec2(-radius, -radius);
	static std::vector<Tile*> tiles;
	tiles.clear();

	for (int x = 0; x <= 2 * radius; ++x){
		for (int y = 0; y <= 2 * radius; ++y){
			Tile& tile = GetTileAtPosition(currentPos);
			if (actor.m_visibleTiles.find(&tile) != actor.m_visibleTiles.cend()){
				if (tile.GetTileType() == type && (!onlyCanBeMovedInto || tile.CanBeMovedInto())){
					tiles.push_back(&tile);
				}
			}

			++currentPos.y;
		}
		currentPos.y = position.y - radius;
		++currentPos.x;
	}

	return tiles;
}

///=====================================================
/// 
///=====================================================
std::vector<Tile*>& Map::GetAllTilesOfType(TileType type, bool onlyCanBeMovedInto /*= false*/) const{
	static std::vector<Tile*> tiles;
	tiles.clear();

	for (std::vector<Tile>::const_iterator tileIter = m_tiles.cbegin(); tileIter != m_tiles.cend(); ++tileIter){
		const Tile& tile = *tileIter;
		if (tile.GetTileType() == type && (!onlyCanBeMovedInto || tile.CanBeMovedInto()))
			tiles.push_back((Tile*)&tile);
	}

	return tiles;
}

///=====================================================
/// 
///=====================================================
void Map::SaveGame(XMLNode& mapNode, XMLNode& mapVisibilityNode) const{
	std::string mapGlyphs = "\n";
	std::string mapVisibility = "\n";

	for (int h = m_size.y - 1; h >= 0; --h){
		for (int w = 0; w < m_size.x; ++w){
			const Tile& tile = m_tiles[h * m_size.x + w];
			mapGlyphs += Tile::TILE_SYMBOL_TABLE[tile.m_tileType];
			if (tile.m_hasBeenSeen)
				mapVisibility += Tile::TILE_SYMBOL_TABLE[tile.m_tileType];
			else
				mapVisibility += '?';
		}
		mapGlyphs += '\n';
		mapVisibility += '\n';
	}

	mapNode.addText(mapGlyphs.c_str());
	mapVisibilityNode.addText(mapVisibility.c_str());
}
