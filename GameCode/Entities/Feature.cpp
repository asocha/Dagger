//=====================================================
// Feature.cpp
// by Andrew Socha
//=====================================================

#include "Feature.hpp"
#include "../Map.hpp"
#include "Engine/XML/XMLParsingSupport.hpp"
#include "Actor.hpp"
#include "../MessageBar.hpp"

Features Feature::s_featuresOnMap;

///=====================================================
/// 
///=====================================================
Feature::Feature(const XMLNode& featureNode, OpenGLRenderer* renderer) :
Entity(featureNode, renderer),
m_type(Undefined),
m_isUsed(false),
m_blocksMovementUsed(false),
m_blocksMovementUnused(false),
m_blocksVisibilityUnused(false),
m_blocksVisibilityUsed(false){
	m_name = GetStringAttribute(featureNode, "name");

	std::string featureType = GetStringAttribute(featureNode, "type");

	RGBAchars defaultColor = RGBAchars::WHITE;
	char defaultGlyph = ':';

	if (featureType == "Door"){
		m_type = FeatureType::Door;

		m_blocksMovementUnused = true;
		m_blocksVisibilityUnused = true;
	}
	else{ //unknown feature
		FATAL_ERROR();
	}

	if (m_type != FeatureType::Undefined){
		defaultColor = FEATURE_COLOR_TABLE[m_type];
		defaultGlyph = FEATURE_SYMBOL_TABLE[m_type];
	}

	m_glyph = GetCharAttribute(featureNode, "glyph", defaultGlyph);
	m_color = GetColorAttribute(featureNode, "color", defaultColor);
	m_textRenderer.SetInputString(std::string(1, m_glyph));
	m_textRenderer.SetInputStringColor(m_color);
}

///=====================================================
/// 
///=====================================================
Feature::Feature(const Feature& other, OpenGLRenderer* renderer, const XMLNode& saveData /*= XMLNode::emptyNode()*/):
Entity(other, renderer, saveData),
m_type(other.m_type),
m_isUsed(other.m_isUsed),
m_blocksMovementUsed(other.m_blocksMovementUsed),
m_blocksMovementUnused(other.m_blocksMovementUnused),
m_blocksVisibilityUnused(other.m_blocksVisibilityUnused),
m_blocksVisibilityUsed(other.m_blocksVisibilityUsed){
	if (!saveData.isEmpty()){
		m_isUsed = GetBoolAttribute(saveData, "isUsed");

		m_glyph = GetCharAttribute(saveData, "glyph", m_glyph);
		m_color = GetColorAttribute(saveData, "color", m_color);
	}

	m_textRenderer.SetInputString(std::string(1, m_glyph));
	m_textRenderer.SetInputStringColor(m_color);
}

///=====================================================
/// 
///=====================================================
void Feature::AddToMap(Map* map, const IntVec2& position){
	FATAL_ASSERT(map != nullptr);

	m_position = position;
	m_map = map;

	Tile& tile = map->GetTileAtPosition(position);

	Feature* tileFeature = tile.m_feature;
	if (tileFeature != nullptr){
		RECOVERABLE_ERROR();
		return;
	}

	tile.m_feature = this;
	s_featuresOnMap.push_back(this);
}

///=====================================================
/// 
///=====================================================
void Feature::Render(OpenGLRenderer* renderer, bool forceVisible /*= false*/){
	if (m_position.x == -1) return;

	const Tile& tile = m_map->GetTileAtPosition(m_position);
	if (tile.m_actor != nullptr && (tile.m_isVisibleToPlayer || !tile.m_hasBeenSeen || forceVisible)) return;

	if (tile.m_feature == nullptr){
		RECOVERABLE_ERROR();
		return;
	}

	if (tile.m_hasBeenSeen){
		if (!tile.m_isVisibleToPlayer && !forceVisible){
			m_textRenderer.SetInputStringColor(RGBAchars::GRAY);
		}
		else{
			m_textRenderer.SetInputStringColor(m_color);
		}
		forceVisible = true;
	}

	if (tile.m_items != nullptr){ //item and feature here
		m_textRenderer.SetInputString("*");
		if (m_textRenderer.GetInputStringColor() != RGBAchars::GRAY)
			m_textRenderer.SetInputStringColor(RGBAchars::WHITE);
	}

	Entity::Render(renderer, forceVisible);

	if (tile.m_items != nullptr){
		m_textRenderer.SetInputString(std::string(1, m_glyph));
		if (m_textRenderer.GetInputStringColor() != RGBAchars::GRAY)
			m_textRenderer.SetInputStringColor(m_color);
	}
}

///=====================================================
/// 
///=====================================================
void Feature::UseFeature(const Actor& user){
	RECOVERABLE_ASSERT(m_isUsed == false);
	RECOVERABLE_ASSERT(user.m_canUseFeatures == true);

	m_isUsed = true;

	if (m_type == Door){
		m_glyph = '-';
		m_textRenderer.SetInputString("-");

		if (user.IsPlayer() || user.CanSeePlayer()){
			std::string myNameToLower = m_name;
			std::transform(m_name.begin(), m_name.end(), myNameToLower.begin(), ::tolower);
			s_theMessageBar->m_messages.push_back(user.m_name + " opened the " + myNameToLower + ".");
		}
	}
}

///=====================================================
/// 
///=====================================================
void Feature::Die(){
	m_map->GetTileAtPosition(m_position).m_feature = nullptr;

	Entity::Die();
}

///=====================================================
/// 
///=====================================================
void Feature::SaveGame(XMLNode& entitiesNode) const{
	XMLNode featureNode = entitiesNode.addChild("Feature");
	Entity::SaveGame(featureNode);

	SetCharAttribute(featureNode, "glyph", m_glyph, FEATURE_SYMBOL_TABLE[m_type]);
	SetColorAttribute(featureNode, "color", m_color, FEATURE_COLOR_TABLE[m_type]);

	SetBoolAttribute(featureNode, "isUsed", m_isUsed);
}
