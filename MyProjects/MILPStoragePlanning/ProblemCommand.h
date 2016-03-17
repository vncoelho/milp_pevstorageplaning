#ifndef MILPStoragePlanning_PROBLEMMODULE_H_
#define MILPStoragePlanning_PROBLEMMODULE_H_

#include <iostream>

#include "../../OptFrame/Scanner++/Scanner.h"

#include "Representation.h"
#include "DeltaStructure.h"
#include "Solution.h"
#include "Evaluation.h"

#include "Evaluator.h"
#include "ConstructiverandomWeights.h"
#include "NSSeqchangeObjWeight.h"

#include "ProblemInstance.h"

#include "../../OptFrame/RandGen.hpp"
#include "../../OptFrame/Util/CheckCommand.hpp"

using namespace scannerpp;
using namespace optframe;

namespace MILPStoragePlanning
{

class MILPStoragePlanningProblemCommand
{
public:

    CheckCommand<RepMILPStoragePlanning, MY_ADS >* check;
    RandGen& rg;
    vector<Component*> vd;
    
    void registerComponent(Component* c, string s)
    {
        vd.push_back(c);
    }

    ProblemInstance* p;
    
    MILPStoragePlanningProblemCommand(RandGen& _rg) 
        : rg(_rg)
    { 
        p = NULL;
        check = NULL;
    }

    virtual ~MILPStoragePlanningProblemCommand()
    {
    }
    
    string id()
    {
        return "problem.MILPStoragePlanning";
    }
    
    // Implement 'load' and 'unload' methods in .cpp file
    bool load(string filename);

    bool unload();
};

}

#endif /*MILPStoragePlanning_PROBLEMMODULE_H_*/

