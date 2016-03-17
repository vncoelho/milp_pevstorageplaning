#ifndef MILPStoragePlanning_NSSEQchangeObjWeight_H_
#define MILPStoragePlanning_NSSEQchangeObjWeight_H_

// Framework includes
#include "../../OptFrame/NSSeq.hpp"
#include "../../OptFrame/RandGen.hpp"

// Own includes
#include "ProblemInstance.h"
#include "DeltaStructure.h"
#include "Solution.h"

using namespace std;

namespace MILPStoragePlanning
{

class MovechangeObjWeight: public Move< RepMILPStoragePlanning , MY_ADS  >
{
private:
    // MOVE PARAMETERS

public:
    using Move< RepMILPStoragePlanning , MY_ADS  >::apply; // prevents name hiding
    using Move< RepMILPStoragePlanning , MY_ADS  >::canBeApplied; // prevents name hiding

    MovechangeObjWeight() // ADD PARAMETERS
    {
    }

    virtual ~MovechangeObjWeight()
    {
    }
    
    void print() const
    {
        cout << id() << " with params: '" << "ADD MY PARAMETERS" << "'" << endl;
    }
    
    string id() const
    {
        return Move<RepMILPStoragePlanning , MY_ADS >::idComponent().append(":MovechangeObjWeight");
    }
    
    bool operator==(const Move< RepMILPStoragePlanning , MY_ADS  >& _m) const
    {
        const MovechangeObjWeight& m = (const MovechangeObjWeight&) _m;
        // COMPARE PARAMETERS AND RETURN TRUE IF EQUALS
        return false;
    }
    
    // Implement these methods in the .cpp file
    
    bool canBeApplied(const RepMILPStoragePlanning& rep, const MY_ADS&);

    Move< RepMILPStoragePlanning , MY_ADS  >* apply(RepMILPStoragePlanning& rep, MY_ADS&);
    
//    MoveCost* cost(const Evaluation<  >&, const RepMILPStoragePlanning& rep, const MY_ADS& ads);
};



class NSIteratorchangeObjWeight: public NSIterator< RepMILPStoragePlanning , MY_ADS  >
{
private:
    // ITERATOR PARAMETERS

public:
    NSIteratorchangeObjWeight() // ADD ITERATOR PARAMETERS
    {
    }

    virtual ~NSIteratorchangeObjWeight()
    {
    }
    
    // Implement these methods in the .cpp file

    void first();
    void next();
    bool isDone();
    Move< RepMILPStoragePlanning , MY_ADS  >& current();
};



class NSSeqchangeObjWeight: public NSSeq< RepMILPStoragePlanning , MY_ADS  >
{
private:
    // YOU MAY REMOVE THESE PARAMETERS IF YOU DON'T NEED (BUT PROBABLY WILL...)
    ProblemInstance& pMILPStoragePlanning; // problem instance data
    RandGen& rg;                // random number generator

public:

    using NSSeq< RepMILPStoragePlanning , MY_ADS  >::move; // prevents name hiding

    // YOU MAY REMOVE THESE PARAMETERS IF YOU DON'T NEED (BUT PROBABLY WILL...)
    NSSeqchangeObjWeight(ProblemInstance& _pMILPStoragePlanning, RandGen& _rg):
        pMILPStoragePlanning(_pMILPStoragePlanning), rg(_rg)
    {
    }

    virtual ~NSSeqchangeObjWeight()
    {
    }
    
    void print() const
    {
        cout << "NSSeqchangeObjWeight" << endl;
    }
    
    string id() const
    {
        return NSSeq<RepMILPStoragePlanning , MY_ADS >::idComponent().append(":NSSeqchangeObjWeight");
    }
    
    NSIterator<RepMILPStoragePlanning , MY_ADS >& getIterator(const RepMILPStoragePlanning& rep, const MY_ADS&)
    {
        // return an iterator to the neighbors of 'rep' 
        return * new NSIteratorchangeObjWeight;  // ADD POSSIBLE ITERATOR PARAMETERS
    }
        
    // Implement this method in the .cpp file

    Move<RepMILPStoragePlanning , MY_ADS >& move(const RepMILPStoragePlanning& rep, const MY_ADS&);
};

}

#endif /*MILPStoragePlanning_NSSEQchangeObjWeight_H_*/
