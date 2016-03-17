#include "Evaluator.h"

using namespace MILPStoragePlanning;

MILPStoragePlanningEvaluator::MILPStoragePlanningEvaluator(ProblemInstance& _pMILPStoragePlanning): // If necessary, add more parameters
pMILPStoragePlanning(_pMILPStoragePlanning)
{
	// Put the rest of your code here
}
	
MILPStoragePlanningEvaluator::~MILPStoragePlanningEvaluator()
{
}

EvaluationMILPStoragePlanning& MILPStoragePlanningEvaluator::evaluate(const RepMILPStoragePlanning& rep)
{
    // 'rep' is the representation of the solution

    double fo = 0; // Evaluation Function Value

    return * new EvaluationMILPStoragePlanning(fo  , * new int);
}

bool MILPStoragePlanningEvaluator::betterThan(double f1, double f2)
{
    // MAXIMIZATION
    return (f1 > (f2 + EPSILON_MILPStoragePlanning));
}

bool MILPStoragePlanningEvaluator::isMinimization() const
{
	// MAXIMIZATION
	return false;
}
