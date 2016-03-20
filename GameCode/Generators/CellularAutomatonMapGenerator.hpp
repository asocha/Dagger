//=====================================================
// CellularAutomatonMapGenerator.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_CellularAutomataMapGenerator__
#define __included_CellularAutomataMapGenerator__

#include "BaseMapGenerator.hpp"

class CellularAutomatonMapGenerator : public BaseMapGenerator{
private:
	static MapGeneratorRegistration s_cellularAutomatonMapGeneratorRegistration;
	inline static BaseMapGenerator* CreateMapGenerator(const std::string& name){ return new CellularAutomatonMapGenerator(name); }

	int m_iterationCount;

public:
	CellularAutomatonMapGenerator(const std::string& name);
	Map* CreateMap(OpenGLRenderer* renderer, const XMLNode& mapNode = XMLNode::emptyNode());
	void ProcessOneStep(Map* map);
	void AutoGenerateMap(Map* map);
};

#endif