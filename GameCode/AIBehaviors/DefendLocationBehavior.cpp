//=====================================================
// DefendLocationBehavior.cpp
// by Andrew Socha
//=====================================================

#include "DefendLocationBehavior.hpp"

//=====================================================
// PickUpItemBehavior.cpp
// by Andrew Socha
//=====================================================

#include "DefendLocationBehavior.hpp"

AIBehaviorRegistration DefendLocationBehavior::s_defendLocationBehaviorRegistration(std::string("DefendLocation"), (AICreationFunction*)&CreateAIBehavior);

///=====================================================
/// 
///=====================================================
DefendLocationBehavior::DefendLocationBehavior(const std::string& name, const XMLNode& behaviorNode) :
PickUpItemBehavior(name, behaviorNode){
}

///=====================================================
/// 
///=====================================================
float DefendLocationBehavior::CalcUtility(){
	return 0.0f; //only AISystem can trigger this behavior
}