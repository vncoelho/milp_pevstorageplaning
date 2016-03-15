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

/*
 * MultObjectiveMetrics.hpp
 *
 * LGPLv3
 *
 */

#ifndef OPTFRAME_MOMETRICS_HPP_
#define OPTFRAME_MOMETRICS_HPP_

#define EPSILON 0.0001

#include "../Solution.hpp"
#include "../Evaluation.hpp"
#include "../Evaluator.hpp"
#include "../Move.hpp"
#include "../Population.hpp"

#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

class UnionNDSets2
{
protected:

	bool addSolution(vector<vector<double> >& p, vector<double>& s)
	{
		bool added = true;
		for (int ind = 0; ind < p.size(); ind++)
		{

			if (weakDominates(p[ind], s))
				return false;

			if (dominates(s, p.at(ind)))
			{
				p.erase(p.begin() + ind); //Esta perdendo o individuo retornado,tem problema? todo
				ind--;
			}
		}

		if (added == true)
			p.push_back(s);

		return added;
	}

	bool addSolutionParetoRate(vector<vector<double> >& p, vector<double>& s, vector<vector<double> >& pRate)
	{
		bool added = true;
		for (int ind = 0; ind < p.size(); ind++)
		{

			if (weakDominates(p[ind], s))
				return false;

			if (dominates(s, p.at(ind)))
			{
				p.erase(p.begin() + ind); //Esta perdendo o individuo retornado,tem problema? todo
				pRate.erase(pRate.begin() + ind); //Esta perdendo o individuo retornado,tem problema? todo
				ind--;
			}
		}

		if (added == true)
			p.push_back(s);

		return added;
	}

	// true if 's1' dominates 's2'
	virtual bool weakDominates(vector<double>& s1, const vector<double>& s2)
	{

		int better = 0;
		int equals = 0;

		for (int e = 0; e < s1.size(); e++)
		{
			if (betterThan(s1[e], s2[e]))
				better++;

			if (abs(s1[e] - s2[e]) < EPSILON)
				equals++;
		}

		return ((better + equals == nObj));
	}

	virtual bool betterThan(double a, double b)
	{
		// MINIMIZATION
		return (a < (b - EPSILON));
	}

	// true if 's1' weak dominates 's2'
	virtual bool dominates(vector<double>& s1, const vector<double>& s2)
	{

		int better = 0;
		int equals = 0;

		for (int e = 0; e < s1.size(); e++)
		{
			if (betterThan(s1[e], s2[e]))
				better++;

			if (abs(s1[e] - s2[e]) < EPSILON)
				equals++;
		}

		return ((better + equals == nObj) && (better > 0));
	}

public:

	int nObj;
	UnionNDSets2(int _nObj)
	{
		nObj = _nObj;

	}

	virtual ~UnionNDSets2()
	{
	}

	vector<vector<double> > unionSets(string caminho, int nTests)
	{

		vector<vector<double> > D;

		for (int e = 0; e < nTests; e++)
		{
			stringstream ss;
			ss << caminho;

			Scanner scanner(new File(ss.str()));
			while (scanner.hasNext())
			{
				double eval1 = scanner.nextDouble();
				double eval2 = scanner.nextDouble();
				double eval3 = scanner.nextDouble();
				vector<double> ind;
				ind.push_back(eval1);
				ind.push_back(eval2);
				ind.push_back(eval3);

				addSolution(D, ind);
			}

		}

		return D;
	}

	vector<vector<double> > unionSets(vector<vector<double> > D1, vector<vector<double> > D2)
	{
		vector<vector<double> > ref = D1;

		for (int ind = 0; ind < D2.size(); ind++)
		{
			bool added = addSolution(ref, D2[ind]);

		}

		return ref;
	}

	void printPareto(vector<double> Pop, int nObj)
	{

		for (int nFO = 0; nFO < nObj; nFO++)
		{
			cout << Pop[nFO] << "\t";
		}
		cout << endl;

	}

	void printVectorPareto(vector<vector<double> > Pop, int nObj)
	{
		for (int nP = 0; nP < Pop.size(); nP++)
		{
			for (int nFO = 0; nFO < nObj; nFO++)
			{
				cout << Pop[nP][nFO] << "\t";
			}
			cout << endl;
		}
	}

	vector<vector<double> > createParetoSet(vector<vector<double> > D2)
	{
		vector<vector<double> > ref;
		ref.push_back(D2[0]);

		for (int ind = 1; ind < D2.size(); ind++)
		{
			bool added = addSolution(ref, D2[ind]);
			/*			if (added)
			 {
			 cout << "Adicionado!" << endl;
			 printVectorPareto(ref,3);
			 cout<<"newSol"<<endl;
			 printPareto(D2[ind], 3);
			 getchar();
			 }*/
		}

		return ref;
	}

	pair<vector<vector<double> >, vector<vector<double> > > createParetoSetSavingRate(vector<vector<double> > D2, vector<vector<double> > PopRate)
	{
		vector<vector<double> > paretoRate;
		vector<vector<double> > ref;
		ref.push_back(D2[0]);
		paretoRate.push_back(PopRate[0]);

		for (int ind = 1; ind < D2.size(); ind++)
		{
			bool added = addSolutionParetoRate(ref, D2[ind], paretoRate);
			if (added)
			{
				paretoRate.push_back(PopRate[ind]);
			}
		}

		return make_pair(ref, paretoRate);
	}

	int cardinalite(vector<vector<double> > D, vector<vector<double> > ref)
	{
		int card = 0;
		for (int i = 0; i < D.size(); i++)
		{
			for (int j = 0; j < ref.size(); j++)
			{
				int equal = 0;
				for (int eval = 0; eval < D[0].size(); eval++)
				{
					if (D[i][eval] == ref[j][eval])
						equal++;
				}
				if (equal == D[0].size())
					card++;
			}
		}

		return card;
	}

	double setCoverage(vector<vector<double> > a, vector<vector<double> > b)
	{
		double cover = 0;

		for (int i = 0; i < b.size(); i++)
		{
			for (int j = 0; j < a.size(); j++)
			{
				bool wD = weakDominates(a[j], b[i]);
				if (wD)
				{
					j = a.size();
					cover += 1;
					//cout<<" i = "<<i<<" j = "<<j<<endl;
					//getchar();
				}
			}
		}

		//cout<<" cover = "<<cover<<endl;

		double sCover = cover / b.size();

		return sCover;
	}

	double deltaMetric(vector<vector<double> > pareto, vector<double> utopicEval)
	{

		vector<double> vDist;

		int nObj = utopicEval.size();
		int nSol = pareto.size();
		vector<double> minEval(nObj, 1000000);
		double dMean = 0;
		for (int nS = 0; nS < nSol; nS++)
		{
			//vector with the evaluation of solution nS
			vector<double> eval = pareto[nS];
			for (int e = 0; e < nObj; e++)
			{
				if (eval[e] < minEval[e])
					minEval[e] = eval[e];
			}

			double minDist = 10000000;
			for (int nS2 = 0; nS2 < pareto.size(); nS2++)
			{
				double dist = 0;
				if (nS2 != nS)
				{
					vector<double> eval2 = pareto[nS2];
					for (int e = 0; e < nObj; e++)
						dist += pow(eval[e] - eval2[e], 2);
					dist = sqrt(dist);
					if (dist < minDist)
						minDist = dist;
				}

			}

			vDist.push_back(minDist);
			dMean += minDist;

		}
		dMean /= nSol;

		double dUtopic = 0;
		for (int e = 0; e < nObj; e++)
		{
			dUtopic += pow(minEval[e] - utopicEval[e], 2);
			//cout<<minEval[e]<<endl;
			//cout<<utopicEval[e]<<endl;
		}

		dUtopic = sqrt(dUtopic);
		//cout<<dUtopic<<endl;
		//getchar();

		double sumDist = 0;
		for (int nS = 0; nS < nSol; nS++)
		{
			sumDist += vDist[nS] - dMean;
		}

		double delta = (dUtopic + sumDist) / (nSol * dMean + dUtopic);
		return delta;
		//cout << "delta = " << delta << endl;
		//getchar();

	}
	double spacing(vector<vector<double> > a)
	{
		double ss = 0;
		vector<double> distMin;
		int N = a.size();
		int m = a[0].size();
		for (int i = 0; i < a.size(); i++)
		{
			double di = 100000000;

			for (int j = 0; j < a.size(); j++)
			{
				if (i != j)
				{
					double diMin = 0;
					for (int eval = 0; eval < m; eval++)
						diMin += abs(a[i][eval] - a[j][eval]);

					if (diMin < di)
						di = diMin;
				}
			}
			distMin.push_back(di);
		}

		double dMean = 0;

		for (int i = 0; i < N; i++)
			dMean += distMin[i];

		dMean = dMean / N;

		for (int i = 0; i < N; i++)
			ss += pow(distMin[i] - dMean, 2);

		ss = ss / N;
		ss = sqrt(ss);

		return ss;
	}

	vector<double> spacing2(vector<vector<vector<double> > > D)
	{
		vector<double> spacings;
		for (int frente = 0; frente < D.size(); frente++)
		{
			vector<vector<double> > a = D[frente];
			double ss = 0;
			vector<double> distMin;
			int N = a.size();
			int m = a[0].size();
			for (int i = 0; i < a.size(); i++)
			{
				double di = 100000000;

				for (int j = 0; j < a.size(); j++)
				{
					if (i != j)
					{
						double diMin = 0;
						for (int eval = 0; eval < m; eval++)
							diMin += abs(a[i][eval] - a[j][eval]);

						if (diMin < di)
							di = diMin;
					}
				}
				distMin.push_back(di);
			}

			double dMean = 0;

			for (int i = 0; i < N; i++)
				dMean += distMin[i];

			dMean = dMean / N;

			for (int i = 0; i < N; i++)
				ss += pow(distMin[i] - dMean, 2);

			ss = ss / N;
			ss = sqrt(ss);
			spacings.push_back(ss);
		}
		return spacings;
	}
};

#endif /*OPTFRAME_MOMETRICS_HPP_*/
