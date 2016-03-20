//=====================================================
// Tile.cpp
// by Andrew Socha
//=====================================================

#include "Tile.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Entities/Actor.hpp"
#include "Entities/Feature.hpp"


std::string Tile::TILE_SYMBOL_TABLE[3] = { "?", "#", "." };

///=====================================================
/// 
///=====================================================
Tile::Tile(const IntVec2& position, TileType type /*= TileType::Air*/) :
m_tileType(type),
m_isVisibleToPlayer(false),
m_hasBeenSeen(false),
m_actor(nullptr),
m_items(nullptr),
m_feature(nullptr),
m_position(position),
m_isCompletelyHidden(false){
}

///=====================================================
/// 
///=====================================================
Tile::~Tile(){
	delete m_items;
	m_items = nullptr;

	delete m_feature;
	m_feature = nullptr;

	delete m_actor;
	m_actor = nullptr;
}

///=====================================================
/// 
///=====================================================
void Tile::SetTileType(TileType newType){
	if (m_tileType == TileType::OutsideOfWorld)
		return;

	m_tileType = newType;
}

///=====================================================
/// 
///=====================================================
TileType Tile::ConvertCharToTileType(char character){
	for (int i = 0; i < TileType::NumTiles; ++i){
		if (TILE_SYMBOL_TABLE[i][0] == character)
			return (TileType)i;
	}

	return TileType::OutsideOfWorld;
}

///=====================================================
/// 
///=====================================================
bool Tile::CanBeMovedInto() const{
	bool canBeMovedInto = (m_tileType == TileType::Air && m_actor == nullptr);
	canBeMovedInto = canBeMovedInto && (m_feature == nullptr || (m_feature->m_blocksMovementUnused == false && !m_feature->m_isUsed) || (m_feature->m_blocksMovementUsed == false && m_feature->m_isUsed));
	return canBeMovedInto;
}

///=====================================================
/// 
///=====================================================
void Tile::Render(Console& textRenderer, const OpenGLRenderer* renderer, float fontSize, const Vec2& offset, bool forceVisible /*= false*/){
	if (!m_hasBeenSeen && !forceVisible) return;
	if ((m_actor != nullptr || m_feature != nullptr || m_items != nullptr) && (m_isVisibleToPlayer || forceVisible)) return;
	if (m_feature != nullptr && m_hasBeenSeen && !forceVisible) return;

	textRenderer.SetInputString(TILE_SYMBOL_TABLE[m_tileType]);
	
	if (m_isVisibleToPlayer || forceVisible){
		textRenderer.SetInputStringColor(RGBAchars::WHITE);
	}
	else{
		textRenderer.SetInputStringColor(RGBAchars::GRAY);
	}

	textRenderer.RenderText(renderer, Font::CONSOLAS, fontSize, offset + Vec2(103.0f + (float)m_position.x * (16.4f / 35.0f) * fontSize, 100.0f + (float)m_position.y * fontSize));
}
