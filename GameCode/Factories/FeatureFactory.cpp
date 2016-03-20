//=====================================================
// FeatureFactory.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#include "FeatureFactory.hpp"
#include "Engine/Core/Utilities.hpp"
#include "Engine/XML/xmlParser.h"
#include "Engine/Core/StringTable.hpp"

FeatureFactoryRegistry FeatureFactory::s_factoriesByName;
std::multimap<unsigned int, FeatureFactory*> FeatureFactory::s_factoriesByType;

///=====================================================
/// 
///=====================================================
FeatureFactory::FeatureFactory(const XMLNode& itemNode, OpenGLRenderer* renderer) :
m_featureBlueprint(nullptr),
m_theRenderer(renderer){
	m_featureBlueprint = new Feature(itemNode, renderer);

	//pull out ranges here
}

///=====================================================
/// 
///=====================================================
FeatureFactory::~FeatureFactory(){
	delete m_featureBlueprint;
}

///=====================================================
/// 
///=====================================================
Feature* FeatureFactory::SpawnFeature(const XMLNode& saveData/* = XMLNode::emptyNode()*/) const{
	return new Feature(*m_featureBlueprint, (OpenGLRenderer*)m_theRenderer, saveData);
}

///=====================================================
/// 
///=====================================================
bool FeatureFactory::LoadAllFeatureFactories(OpenGLRenderer* renderer){
	std::vector<std::string> factories;
	bool foundFiles = FindAllFilesOfType("Data/Features/", "*.feature.xml", factories);
	RECOVERABLE_ASSERT(foundFiles == true);

	for (std::vector<std::string>::const_iterator factoryIter = factories.cbegin(); factoryIter != factories.cend(); ++factoryIter){
		XMLNode featuresNode = XMLNode::parseFile(factoryIter->c_str());
		XMLNode featureNode = featuresNode.getChildNode(0);
		int childNum = 1;

		while (!featureNode.isEmpty()){
			FeatureFactory* newFactory = new FeatureFactory(featureNode, renderer);

			s_factoriesByName.emplace(GetStringID(newFactory->m_featureBlueprint->m_name), newFactory);
			s_factoriesByType.emplace(newFactory->m_featureBlueprint->m_type, newFactory);

			featureNode = featuresNode.getChildNode(childNum++);
		}
	}

	return foundFiles;
}

///=====================================================
/// 
///=====================================================
void FeatureFactory::DeleteFeatureFactories(){
	for (FeatureFactoryRegistry::const_iterator factoryIter = FeatureFactory::s_factoriesByName.cbegin(); factoryIter != FeatureFactory::s_factoriesByName.cend(); ++factoryIter){
		delete factoryIter->second;
	}
}

///=====================================================
/// 
///=====================================================
FeatureFactory* FeatureFactory::FindFactoryByName(const std::string& name){
	FeatureFactoryRegistry::iterator factoryIter = s_factoriesByName.find(GetStringID(name));
	if (factoryIter == s_factoriesByName.end())
		return nullptr;
	return factoryIter->second;
}

///=====================================================
/// 
///=====================================================
FeatureFactory* FeatureFactory::FindFactoryByType(Feature::FeatureType type){
	std::pair<std::multimap<unsigned int, FeatureFactory*>::iterator, std::multimap<unsigned int, FeatureFactory*>::iterator> factoryRange = s_factoriesByType.equal_range(type);
	if (factoryRange.first == s_factoriesByType.end()){
		RECOVERABLE_ERROR();
		return nullptr;
	}

	std::multimap<unsigned int, FeatureFactory*>::iterator firstFactoryOfType = factoryRange.first;

	int numFactories = 0;
	for (; factoryRange.first != factoryRange.second; ++factoryRange.first, ++numFactories){}

	int factoryNum = GetRandomIntLessThan(numFactories); //get a random factory of this type
	for (int i = 0; i < factoryNum; ++i){
		++firstFactoryOfType;
	}

	return firstFactoryOfType->second;
}
