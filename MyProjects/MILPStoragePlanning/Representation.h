#ifndef MILPStoragePlanning_REPRESENTATION_H_
#define MILPStoragePlanning_REPRESENTATION_H_

#include <vector>
#include "../../OptFrame/Util/printable.h"

#include "../../OptFrame/Util/Matrix.hpp"

using namespace std;

struct RepMILPStoragePlanning
{
	vector<vector<vector<bool> > > pevsDischarge;
	vector<vector<vector<bool> > > pevsCharge;

	double objCost;
	double objWearTear ;
	double objMaxPeakLoad ;

};
// Solution Representation

//#include <ostream>
ostream & operator<<(ostream & s, RepMILPStoragePlanning)
{
}

#endif /*MILPStoragePlanning_REPRESENTATION_H_*/

