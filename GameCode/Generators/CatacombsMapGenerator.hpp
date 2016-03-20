//=====================================================
// CatacombsMapGenerator.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_CatacombsMapGenerator__
#define __included_CatacombsMapGenerator__

#include "BaseMapGenerator.hpp"

class CatacombsMapGenerator : public BaseMapGenerator{
private:
	static MapGeneratorRegistration s_catacombsMapGeneratorRegistration;
	inline static BaseMapGenerator* CreateMapGenerator(const std::string& name){ return new CatacombsMapGenerator(name); }

	void PopulateMap(Map* map, const XMLNode& entitiesNode = XMLNode::emptyNode());

	int m_currentStep;
	IntVec2 m_vampireHallway;
	IntVec2 m_entranceHallway;

public:
	CatacombsMapGenerator(const std::string& name);
	Map* CreateMap(OpenGLRenderer* renderer, const XMLNode& mapNode = XMLNode::emptyNode());
	void ProcessOneStep(Map* map);
	void AutoGenerateMap(Map* map);
};

#endif