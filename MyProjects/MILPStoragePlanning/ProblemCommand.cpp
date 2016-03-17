#include "ProblemCommand.h"

using namespace MILPStoragePlanning;

bool MILPStoragePlanningProblemCommand::load(string filename)
{
    File* file;

    try
    {
        file = new File(filename);
    }
    catch (FileNotFound& f)
    {
        cout << "File '" << filename <<"' not found" << endl;
        return false;
    }

    Scanner scanner(file);

    // STEP 1: IMPLEMENT ProblemInstance.h/.cpp (YOU MUST READ THE PROBLEM DATA AND STORE INSIDE)
    p = new ProblemInstance(scanner);

    // OPTIONAL: FOR TESTING
    check = new CheckCommand<RepMILPStoragePlanning, MY_ADS >;

    // STEP 2: IMPLEMENT Evaluator.h/.cpp (THIS IS YOUR EVALUATION FUNCTION)
    registerComponent(new MILPStoragePlanningEvaluator(*p), "myEvaluator");

    // STEP 3: IMPLEMENT ConstructiverandomWeights.h/.cpp (THIS IS YOUR CONSTRUCTIVE METHOD)
    registerComponent(new ConstructiverandomWeights(*p), "constructive_randomWeights");
    
    // STEP 3.1 (EXERCISE): DO THE SAME FOR OTHER ALTERNATIVE CONSTRUCTIVE METHODS (REMEMBER TO ADD THE .cpp IN THE makefile)
    // ...
    
    // STEP 4: IMPLEMENT NSSeqchangeObjWeight.h/.cpp (THIS IS YOUR NEIGHBORHOOD STRUCTURE, YOU HAVE TO IMPLEMENT THE CLASSES: Move, NSIterator and NSSeq)
    registerComponent(new NSSeqchangeObjWeight(*p, rg), "nsseq_changeObjWeight");

    // STEP 4.1 (EXERCISE): DO THE SAME FOR OTHER ALTERNATIVE NEIGHBORHOOD STRUCTURES (REMEMBER TO ADD THE .cpp IN THE makefile)
    // ...

    cout << "problem '" << filename << "' loaded successfully" << endl;
        
    return true;
}

bool MILPStoragePlanningProblemCommand::unload()
{
    if(p)
        delete p;
    p = NULL;
       
    cout << "problem instance for MILPStoragePlanning unloaded successfully (use 'drop_all' if you want to remove all components)" << endl;
       
    return true;
}
