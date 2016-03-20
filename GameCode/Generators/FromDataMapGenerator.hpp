//=====================================================
// FromDataMapGenerator.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_FromDataMapGenerator__
#define __included_FromDataMapGenerator__

#include "BaseMapGenerator.hpp"

class FromDataMapGenerator : public BaseMapGenerator{
private:
	static MapGeneratorRegistration s_fromDataMapGeneratorRegistration;
	inline static BaseMapGenerator* CreateMapGenerator(const std::string& name){ return new FromDataMapGenerator(name); }

	void PopulateMap(Map* map, const XMLNode& entitiesNode = XMLNode::emptyNode());

public:
	FromDataMapGenerator(const std::string& name);
	Map* CreateMap(OpenGLRenderer* renderer, const XMLNode& mapNode = XMLNode::emptyNode());
	void ProcessOneStep(Map* map);
	void AutoGenerateMap(Map* map);
	void FinalizeMap(Map* map, const XMLNode& entitiesNode = XMLNode::emptyNode());
};

#endif