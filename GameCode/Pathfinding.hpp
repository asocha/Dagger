//=====================================================
// Pathfinding.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_Pathfinding__
#define __included_Pathfinding__

#include "Engine/Math/IntVec2.hpp"
class Map;
#include <map>
#include <queue>
class Actor;

struct PathNode{
public:
	float g;
	float f;
	PathNode* m_parent;
	IntVec2 m_position;

	inline PathNode(){}
};

struct Path{
public:
	IntVec2 m_goal;
	std::deque<PathNode*> m_path;
	std::multimap<float, PathNode*> m_openList;
	std::map<IntVec2, PathNode*> m_closedList;

	inline Path() :m_path(), m_openList(), m_closedList(), m_goal(-1, -1){}
	~Path();

	void Reset();
	const std::deque<PathNode*>& CalcPath(bool onlyVisibleTiles, const Actor* actor, const IntVec2& start, const IntVec2& goal, Map* map, bool doActorsBlock = false, bool calcFullPath = true, bool doFeaturesBlock = true);
	bool HasFinishedPath() const;
};


///=====================================================
/// 
///=====================================================
inline bool Path::HasFinishedPath() const{
	return (m_path.empty() || (m_openList.empty() && !m_closedList.empty()));
}


#endif