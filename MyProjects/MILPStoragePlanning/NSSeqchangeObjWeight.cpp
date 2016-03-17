#include "NSSeqchangeObjWeight.h"

using namespace MILPStoragePlanning;

// ============ MovechangeObjWeight ==============

bool MovechangeObjWeight::canBeApplied(const RepMILPStoragePlanning& rep, const MY_ADS&)
{
    return true;
}

Move< RepMILPStoragePlanning , MY_ADS  >& MovechangeObjWeight::apply(RepMILPStoragePlanning& rep, MY_ADS&)
{
    // apply this move to 'rep'
    // rep. (...) = (...)
    // return reverse move
    return * new MovechangeObjWeight; 
}

MoveCost* MovechangeObjWeight::cost(const Evaluation<  >&, const RepMILPStoragePlanning& rep, const MY_ADS& ads)
{
   return NULL;
   // Implement if a more efficient evaluation of the move is available
   //double move_cost = ...;
   //double move_cost_infeasible = ...;
   //return new MoveCost(move_cost, move_cost_infeasible);
}



// ============ NSIteratorchangeObjWeight ==============

void NSIteratorchangeObjWeight::first(){};

void NSIteratorchangeObjWeight::next(){};
	
bool NSIteratorchangeObjWeight::isDone(){};
	
Move< RepMILPStoragePlanning , MY_ADS  >& NSIteratorchangeObjWeight::current(){};




// ============ NSSeqchangeObjWeight ==============


Move<RepMILPStoragePlanning , MY_ADS >& NSSeqchangeObjWeight::move(const RepMILPStoragePlanning& rep, const MY_ADS&)
{
   // return a random move (that is, a move operator that generates a neighbor solution of 'rep')
   // you may need to use the random number generator 'rg'
   
   return * new MovechangeObjWeight; 
}
