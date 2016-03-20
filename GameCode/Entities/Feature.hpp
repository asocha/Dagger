//=====================================================
// Feature.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Feature__
#define __included_Feature__

#include "Entity.hpp"
class Actor;

/**
* Features are either "used" or "unused"
*
* Door: Open = used, Closed = unused
*
*/


typedef std::vector<class Feature*> Features;

class Feature : public Entity{
public:
	enum FeatureType{
		Undefined = -1,
		Door,
		NumFeatureTypes
	};

	FeatureType m_type;

	bool m_isUsed;
	bool m_blocksMovementUsed;
	bool m_blocksMovementUnused;
	bool m_blocksVisibilityUsed;
	bool m_blocksVisibilityUnused;

	static Features s_featuresOnMap;

	Feature(const XMLNode& featureNode, OpenGLRenderer* renderer);
	Feature(const Feature& other, OpenGLRenderer* renderer, const XMLNode& saveData = XMLNode::emptyNode());
	inline ~Feature(){}
	
	void AddToMap(Map* map, const IntVec2& position);
	void Render(OpenGLRenderer* renderer, bool forceVisible = false);

	void UseFeature(const Actor& user);

	void Die();

	void SaveGame(XMLNode& entitiesNode) const;
};

const char FEATURE_SYMBOL_TABLE[Feature::NumFeatureTypes]{'+'};
const RGBAchars FEATURE_COLOR_TABLE[Feature::NumFeatureTypes]{RGBAchars(255, 255, 255, 255)};

#endif