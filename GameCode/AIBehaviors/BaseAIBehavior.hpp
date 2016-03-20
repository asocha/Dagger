//=====================================================
// BaseAIBehavior.hpp
// by Andrew Socha
//=====================================================

#pragma once

#ifndef __included_BaseAIBehavior__
#define __included_BaseAIBehavior__

#include "Engine/Core/EngineCore.hpp"
class NPC;
#include "Engine/XML/xmlParser.h"
#include <string>
#include <map>

enum BehaviorType{
	Attack,
	Retreat,
	Aide,
	OtherBehavior
};
typedef class BaseAIBehavior* (AICreationFunction)(const std::string& name, const XMLNode& behaviorNode);
typedef std::map<std::string, class AIBehaviorRegistration*> BehaviorRegistry;

class AIBehaviorRegistration{
private:
	std::string m_name;
	AICreationFunction* m_creationFunc;
	static BehaviorRegistry* s_registry;

public:
	inline AIBehaviorRegistration(const std::string& name, AICreationFunction* creationFunc)
		:m_creationFunc(creationFunc),
		m_name(name){
		if (s_registry == nullptr){
			s_registry = new BehaviorRegistry();
		}
		s_registry->emplace(name, this);
	}

	inline const std::string& GetName() const{ return m_name; }
	inline static BehaviorRegistry* GetBehaviorRegistry() { return s_registry; }
	inline BaseAIBehavior* CreateNewAIBehavior(const std::string& name, const XMLNode& behaviorNode) const{ return (*m_creationFunc)(name, behaviorNode); }
};

class BaseAIBehavior{
public:
	std::string m_name;
	BehaviorType m_behaviorType;
	NPC* m_NPC;
	XMLNode m_behaviorNode;

	inline BaseAIBehavior(const std::string& name, BehaviorType type, const XMLNode& behaviorNode) :m_NPC(nullptr), m_name(name), m_behaviorType(type), m_behaviorNode(behaviorNode){}
	inline virtual ~BaseAIBehavior(){}

	virtual float CalcUtility() = 0;
	virtual bool Think() = 0;
};

#endif