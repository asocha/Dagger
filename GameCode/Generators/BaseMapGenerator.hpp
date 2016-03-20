//=====================================================
// BaseMapGenerator.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_BaseMapGenerator__
#define __included_BaseMapGenerator__

#include "Engine/Core/EngineCore.hpp"
#include <string>
#include <map>
class Map;
class OpenGLRenderer;
#include "Engine/Math/IntVec2.hpp"
#include "Engine/XML/xmlParser.h"

typedef class BaseMapGenerator*(MapGeneratorCreationFunc)(const std::string& name);
typedef std::map<std::string, class MapGeneratorRegistration*> MapGeneratorRegistry;

class MapGeneratorRegistration{
private:
	std::string m_name;
	MapGeneratorCreationFunc* m_creationFunc;
	static MapGeneratorRegistry* s_registry;

public:
	inline MapGeneratorRegistration(const std::string& name, MapGeneratorCreationFunc* creationFunc)
	:m_creationFunc(creationFunc),
	m_name(name){
		if (s_registry == nullptr){
			s_registry = new MapGeneratorRegistry();
		}
		s_registry->emplace(name, this);
	}

	inline const std::string& GetName() const{ return m_name; }
	inline static MapGeneratorRegistry* GetGeneratorRegistry() { return s_registry; }
	inline BaseMapGenerator* CreateMapGenerator(const std::string& name) const{ return (*m_creationFunc)(name); }
};

class BaseMapGenerator{
protected:
	virtual void PopulateMap(Map* map, const XMLNode& entitiesNode = XMLNode::emptyNode());
	void SurroundMapWithWalls(Map* map);
	void MarkCompletelyHiddenTiles(Map* map);

public:
	std::string m_name;
	IntVec2 m_playerStartPosition;

	inline BaseMapGenerator(const std::string& name) :m_name(name), m_playerStartPosition(-1, -1){}
	inline virtual ~BaseMapGenerator(){}
	
	virtual Map* CreateMap(OpenGLRenderer* renderer, const XMLNode& mapNode = XMLNode::emptyNode()) = 0;
	virtual void ProcessOneStep(Map* map) = 0;
	virtual void AutoGenerateMap(Map* map) = 0;
	virtual void FinalizeMap(Map* map, const XMLNode& entitiesNode = XMLNode::emptyNode());
};

#endif