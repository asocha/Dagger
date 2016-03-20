//=====================================================
// DungeonMapGenerator.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_DungeonGenerator__
#define __included_DungeonGenerator__

#include "BaseMapGenerator.hpp"

class DungeonMapGenerator : public BaseMapGenerator{
private:
	static MapGeneratorRegistration s_dungeonMapGeneratorRegistration;
	inline static BaseMapGenerator* CreateMapGenerator(const std::string& name){ return new DungeonMapGenerator(name); }

public:
	DungeonMapGenerator(const std::string& name);
	Map* CreateMap(OpenGLRenderer* renderer, const XMLNode& mapNode = XMLNode::emptyNode());
	void ProcessOneStep(Map* map);
	void AutoGenerateMap(Map* map);
};

#endif