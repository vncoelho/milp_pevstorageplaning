#ifndef MILPStoragePlanning_EVALUATOR_H_
#define MILPStoragePlanning_EVALUATOR_H_

//#include <cmath>
#include <stdlib.h>
#include <math.h>

#include "../../OptFrame/Evaluation.hpp"
#include "../../OptFrame/Evaluator.hpp"

#include "Representation.h"
#include "DeltaStructure.h"
#include "Solution.h"
#include "Evaluation.h"

#include "ProblemInstance.h"

#define EPSILON_MILPStoragePlanning 0.0001


namespace MILPStoragePlanning
{

class MILPStoragePlanningEvaluator: public Evaluator<RepMILPStoragePlanning, MY_ADS >
{
private:
	ProblemInstance& pMILPStoragePlanning;

	// Your private vars

public:
	MILPStoragePlanningEvaluator(ProblemInstance& _pMILPStoragePlanning);
	
	virtual ~MILPStoragePlanningEvaluator();

	EvaluationMILPStoragePlanning& evaluate(const RepMILPStoragePlanning& rep);
	
	//EvaluationMILPStoragePlanning& evaluate(const RepMILPStoragePlanning& rep, const MY_ADS&);

	virtual bool betterThan(double f1, double f2);
	
	virtual bool isMinimization() const;
};

}

#endif /*MILPStoragePlanning_EVALUATOR_H_*/

