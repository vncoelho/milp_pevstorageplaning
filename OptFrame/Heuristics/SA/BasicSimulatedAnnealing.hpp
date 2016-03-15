// OptFrame - Optimization Framework

// Copyright (C) 2009, 2010, 2011
// http://optframe.sourceforge.net/
//
// This file is part of the OptFrame optimization framework. This framework
// is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License v3 as published by the
// Free Software Foundation.

// This framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License v3 for more details.

// You should have received a copy of the GNU Lesser General Public License v3
// along with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

#ifndef OPTFRAME_BSA_HPP_
#define OPTFRAME_BSA_HPP_

#include "../../SingleObjSearch.hpp"
#include <math.h>

#include "SA.h"

namespace optframe
{

template<class R, class ADS = OPTFRAME_DEFAULT_ADS, class DS = OPTFRAME_DEFAULT_DS>
class BasicSimulatedAnnealing: public SingleObjSearch<R, ADS, DS>
{
private:
	Evaluator<R, ADS, DS>& evaluator;
	Constructive<R, ADS>& constructive;
	vector<NS<R, ADS, DS>*> neighbors;
	RandGen& rg;
	double alpha;
	int SAmax;
	double Ti;

public:

	BasicSimulatedAnnealing(Evaluator<R, ADS, DS>& _evaluator, Constructive<R, ADS>& _constructive, vector<NS<R, ADS, DS>*> _neighbors, double _alpha, int _SAmax, double _Ti, RandGen& _rg) :
		evaluator(_evaluator), constructive(_constructive), neighbors(_neighbors), rg(_rg)
	{
		alpha = (_alpha);
		SAmax = (_SAmax);
		Ti = (_Ti);

	}

	BasicSimulatedAnnealing(Evaluator<R, ADS, DS>& _evaluator, Constructive<R, ADS>& _constructive, NS<R, ADS, DS>* _neighbors, double _alpha, int _SAmax, double _Ti, RandGen& _rg) :
		evaluator(_evaluator), constructive(_constructive), rg(_rg)
	{
		neighbors.push_back(_neighbors);
		alpha = (_alpha);
		SAmax = (_SAmax);
		Ti = (_Ti);
	}

	virtual ~BasicSimulatedAnnealing()
	{
	}

	pair<Solution<R, ADS>&, Evaluation<DS>&>* search(double timelimit = 100000000, double target_f = 0, const Solution<R, ADS>* _s = NULL,  const Evaluation<DS>* _e = NULL)
	{
		cout << "SA search(" << target_f << "," << timelimit << ")" << endl;

		Timer tnow;

		Solution<R, ADS>& s = constructive.generateSolution();
		Evaluation<DS>& e = evaluator.evaluate(s);

		double T = Ti;
		int iterT = 0;
		Solution<R, ADS>* sStar = &s.clone();
		Evaluation<DS>* eStar = &e.clone();

		while ((T > 0) && (tnow.now() < timelimit))
		{
			while ((iterT < SAmax) && (tnow.now() < timelimit))
			{
				int n = rg.rand(neighbors.size());
				Move<R, ADS, DS>* move = &(neighbors[n]->move(s));

				while (!(move->canBeApplied(s)))
				{
					delete move;
					move = &(neighbors[n]->move(s));
				}

				Solution<R, ADS>* sCurrent = &s.clone();
				Evaluation<DS>* eCurrent = &e.clone();
				delete &move->apply(*eCurrent, *sCurrent);
				evaluator.evaluate(*eCurrent, *sCurrent);

				if (evaluator.betterThan(*eCurrent, e))
				{
					e = *eCurrent;
					s = *sCurrent;
					delete sCurrent;
					delete eCurrent;

					if (evaluator.betterThan(e, *eStar))
					{
						delete sStar;
						sStar = &s.clone();
						delete eStar;
						eStar = &e.clone();

						cout << "Best fo: " << e.evaluation() << " Found on Iter = " << iterT << " and T = " << T;
						cout << endl;
					}
				}
				else
				{
					double x = rg.rand01();
					double delta = fabs(eCurrent->evaluation() - e.evaluation());

					if (x < exp(-delta / T))
					{
						s = *sCurrent;
						e = *eCurrent;
						delete sCurrent;
						delete eCurrent;
					}
					else
					{
						delete sCurrent;
						delete eCurrent;
					}
				}

				iterT++;
			}
			T = alpha * T;
			iterT = 0;
		}

		s = *sStar;
		e = *eStar;
		delete sStar;
		delete eStar;

		return new pair<Solution<R, ADS>&, Evaluation<DS>&> (s, e);
	}

	virtual string id() const
	{
		return idComponent();
	}

	static string idComponent()
	{
		stringstream ss;
		ss << SingleObjSearch<R, ADS, DS>::idComponent() << "SA:basic_sa";
		return ss.str();

	}

};

template<class R, class ADS = OPTFRAME_DEFAULT_ADS, class DS = OPTFRAME_DEFAULT_DS>
class BasicSimulatedAnnealingBuilder: public ILS, public SingleObjSearchBuilder<R, ADS, DS>
{
public:
	virtual ~BasicSimulatedAnnealingBuilder()
	{
	}

	virtual SingleObjSearch<R, ADS, DS>* build(Scanner& scanner, HeuristicFactory<R, ADS, DS>& hf, string family = "")
	{
		Evaluator<R, ADS, DS>* eval;
		hf.assign(eval, scanner.nextInt(), scanner.next()); // reads backwards!

		Constructive<R, ADS>* constructive;
		hf.assign(constructive, scanner.nextInt(), scanner.next()); // reads backwards!

		vector<NS<R, ADS, DS>* > hlist;
		hf.assignList(hlist, scanner.nextInt(), scanner.next()); // reads backwards!

		double alpha = scanner.nextDouble();
		int SAmax = scanner.nextInt();
		double Ti = scanner.nextDouble();

		return new BasicSimulatedAnnealing<R, ADS, DS> (*eval, *constructive, hlist, alpha, SAmax, Ti, hf.getRandGen());
	}

	virtual vector<pair<string, string> > parameters()
	{
		vector<pair<string, string> > params;
		params.push_back(make_pair(Evaluator<R, ADS, DS>::idComponent(), "evaluation function"));
		params.push_back(make_pair(Constructive<R, ADS>::idComponent(), "constructive heuristic"));
		params.push_back(make_pair(NS<R, ADS, DS>::idComponent(), "neighborhood structure"));
		params.push_back(make_pair("double", "max number of iterations without improvement"));
		params.push_back(make_pair("int", "max number of iterations without improvement"));
		params.push_back(make_pair("double", "max number of iterations without improvement"));

		return params;
	}

	virtual bool canBuild(string component)
	{
		return component == BasicSimulatedAnnealing<R, ADS, DS>::idComponent();
	}

	static string idComponent()
	{
		stringstream ss;
		ss << SingleObjSearchBuilder<R, ADS, DS>::idComponent() << SA::family() << "BasicSA";
		return ss.str();
	}

	virtual string id() const
	{
		return idComponent();
	}
};

/*

 Procedure originally proposed by Kirkpatrick et al.

 S. Kirkpatrick, D.C. Gellat, and M.P. Vecchi. Optimization by Simulated Annealing.
 Science, 220:671–680, 1983.

 LATEX:

 */

}

#endif /*OPTFRAME_BSA_HPP_*/
