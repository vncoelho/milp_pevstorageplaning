#ifndef MILPStoragePlanning_CONSTRUCTIVE_randomWeights_H_
#define MILPStoragePlanning_CONSTRUCTIVE_randomWeights_H_

#include "../../OptFrame/Constructive.h"
#include "../../OptFrame/Util/TestSolution.hpp"

#include "ProblemInstance.h"

#include "Representation.h"
#include "Solution.h"

#include "Evaluator.h"

#include <list>

#include <algorithm>
#include <stdlib.h>

using namespace std;

using namespace optframe;

namespace MILPStoragePlanning
{

class ConstructiverandomWeights: public Constructive<RepMILPStoragePlanning, MY_ADS>
{
private:
   ProblemInstance& pMILPStoragePlanning;

   // Your private vars

public:
	
	ConstructiverandomWeights(ProblemInstance& _pMILPStoragePlanning);
	
	virtual ~ConstructiverandomWeights();

	Solution<RepMILPStoragePlanning, MY_ADS>& generateSolution();
};

}

#endif /*MILPStoragePlanning_CONTRUCTIVE_randomWeights_H_*/
