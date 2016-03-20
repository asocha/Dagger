//=====================================================
// FeatureFactory.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_FeatureFactory__
#define __included_FeatureFactory__

#include "../Entities/Feature.hpp"
struct XMLNode;
class OpenGLRenderer;

#include <map>

typedef std::map<unsigned int, class FeatureFactory*> FeatureFactoryRegistry;

class FeatureFactory{
private:
	const Feature* m_featureBlueprint;
	const OpenGLRenderer* m_theRenderer;

public:
	FeatureFactory(const XMLNode& featureNode, OpenGLRenderer* renderer);
	~FeatureFactory();

	static FeatureFactoryRegistry s_factoriesByName;
	static std::multimap<unsigned int, FeatureFactory*> s_factoriesByType;

	Feature* SpawnFeature(const XMLNode& saveData = XMLNode::emptyNode()) const;

	static bool LoadAllFeatureFactories(OpenGLRenderer* renderer);
	static void DeleteFeatureFactories();
	static FeatureFactory* FindFactoryByName(const std::string& name);
	static FeatureFactory* FindFactoryByType(Feature::FeatureType type);
};

#endif