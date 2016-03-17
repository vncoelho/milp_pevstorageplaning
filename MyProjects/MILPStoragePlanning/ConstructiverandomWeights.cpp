#include "ConstructiverandomWeights.h"

using namespace MILPStoragePlanning;

ConstructiverandomWeights::ConstructiverandomWeights(ProblemInstance& _pMILPStoragePlanning): 
        pMILPStoragePlanning(_pMILPStoragePlanning)
{
}
	
ConstructiverandomWeights::~ConstructiverandomWeights()
{
}

Solution<RepMILPStoragePlanning, MY_ADS>& ConstructiverandomWeights::generateSolution()
{
    RepMILPStoragePlanning newRep;

    return * new TestSolution<RepMILPStoragePlanning, MY_ADS>(newRep);			
}
