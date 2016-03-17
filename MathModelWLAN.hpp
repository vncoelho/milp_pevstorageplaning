/*
 * OptimalLinearRegression.hpp
 *
 *  Created on: 26/10/2014
 *      Author: vitor
 */

#ifndef MATHMODELWLAN_HPP_
#define MATHMODELWLAN_HPP_

#include <ilcplex/ilocplex.h>
#include <ilcplex/cplexx.h>
#include <ilcplex/ilocplexi.h>

#include <numeric>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>      // std::ifstream
#include <sstream>
#include "OptFrame/Util/MultiObjectiveMetrics2.hpp"
#include "OptFrame/RandGen.hpp"
#include "OptFrame/Timer.hpp"
#include "MyProjects/MILPStoragePlanning/Representation.h"
#include "MyProjects/MILPStoragePlanning/Solution.h"

#include <vector>

ILOSTLBEGIN

using namespace std;
using namespace optframe;

namespace MILPStoragePlanning
{

char* execCommand(const char* command)
{

	FILE* fp;
	char* line = NULL;
	// Following initialization is equivalent to char* result = ""; and just
	// initializes result to an empty string, only it works with
	// -Werror=write-strings and is so much less clear.
	char* result = (char*) calloc(1, 1);
	size_t len = 0;

	fflush(NULL);
	fp = popen(command, "r");
	if (fp == NULL)
	{
		printf("Cannot execute command:\n%s\n", command);
		return NULL;
	}

	while (getline(&line, &len, fp) != -1)
	{
		// +1 below to allow room for null terminator.
		result = (char*) realloc(result, strlen(result) + strlen(line) + 1);
		// +1 below so we copy the final null terminator.
		strncpy(result + strlen(result), line, strlen(line) + 1);
		free(line);
		line = NULL;
	}

	fflush(fp);
	if (pclose(fp) != 0)
	{
		perror("Cannot close stream.\n");
	}

	return result;
}

double hipervolume(vector<vector<double> > v)
{
	int nSol = v.size();
	int nObj = v[0].size();
	string tempFile = "tempFileHipervolueFunc";
	FILE* fTempHV = fopen(tempFile.c_str(), "w");

	for (int s = 0; s < nSol; s++)
	{
		for (int o = 0; o < nObj; o++)
		{
			fprintf(fTempHV, "%.7f\t", v[s][o]);
		}
		fprintf(fTempHV, "\n");
	}

	fclose(fTempHV);
	stringstream ss;
	ss << "./hv\t -r \"" << 10000 << " " << 1000 << " " << 1000 << "\" \t" << tempFile.c_str();
	string hvValueString = execCommand(ss.str().c_str());
	double hvValue = atof(hvValueString.c_str());
	return hvValue;
}

struct TupleMILPCoef
{
	double coefTotalCost, coefMaxLoad, coefWearTear;

	TupleMILPCoef(double _coefTotalCost, double _coefWearTear, double _coefMaxLoad) :
			coefTotalCost(_coefTotalCost), coefMaxLoad(_coefMaxLoad), coefWearTear(_coefWearTear)
	{
	}
};

struct MIPStartSolution
{
	vector<vector<vector<bool> > > pevsDischarge;
	vector<vector<vector<bool> > > pevsCharge;

	double objCost;
	double objWearTear;
	double objMaxPeakLoad;

	MIPStartSolution(vector<vector<vector<bool> > > _pevsDischarge, vector<vector<vector<bool> > > _pevsCharge, double _objCost, double _objMaxPeakLoad, double _objWearTear) :
			pevsDischarge(_pevsDischarge), pevsCharge(_pevsCharge), objCost(_objCost), objMaxPeakLoad(_objMaxPeakLoad), objWearTear(_objWearTear)
	{
	}
};

class mathModelWLAN
{

public:
	RandGen& rg;

	mathModelWLAN(RandGen& _rg) :
			rg(_rg)

	{

	}

	~mathModelWLAN()
	{

	}

	void analyzeParetoFronts(string outputPF1, int sizePF1, string outputPF2, int sizePF2)
	{
		int nObj = 3;
		UnionNDSets2 uND(nObj);
		//===================================
		// TEST PARETO

		vector<vector<double> > PF1 = uND.unionSets(outputPF1.c_str(), sizePF1);
		vector<vector<double> > PF2 = uND.unionSets(outputPF2.c_str(), sizePF2);
		vector<vector<double> > ref = uND.unionSets(PF1, PF2);
		cout << PF1.size() << endl;
		cout << PF2.size() << endl;
		cout << ref.size() << endl;
		int card = uND.cardinalite(PF1, ref);
		double sCToRef = uND.setCoverage(PF1, ref);
		double sCFromRef = uND.setCoverage(ref, PF1);
		vector<double> utopicSol;
		utopicSol.push_back(-10000);
		utopicSol.push_back(0);
		utopicSol.push_back(0);
		//Delta Metric and Hipervolume need to verify min
		cout << "Cardinalite = " << uND.cardinalite(PF1, ref) << endl;
		cout << "Cardinalite = " << uND.cardinalite(PF2, ref) << endl;
		cout << "uND.setCoverage(PF1, ref)  = " << uND.setCoverage(PF1, ref) << endl;
		cout << "uND.setCoverage(PF2, ref)  = " << uND.setCoverage(PF2, ref) << endl;
		cout << "uND.setCoverage(PF2, PF1)  = " << uND.setCoverage(PF2, PF1) << endl;
		cout << "uND.setCoverage(PF1, PF2)  = " << uND.setCoverage(PF1, PF2) << endl;
		cout << "delta  = " << uND.deltaMetric(ref, utopicSol) << endl;
		cout << "delta  = " << uND.deltaMetric(PF1, utopicSol) << endl;
		cout << "delta  = " << uND.deltaMetric(PF2, utopicSol) << endl;
		cout << "hv  = " << hipervolume(ref) << endl;
		cout << "hv  = " << hipervolume(PF1) << endl;
		cout << "hv  = " << hipervolume(PF2) << endl;

		cout << "analises done with sucess!" << endl;
		//============
	}
	void printVectorOfVector(vector<vector<double> > Pop)
	{
		int nObj = Pop[0].size();
		for (int nP = 0; nP < Pop.size(); nP++)
		{
			for (int nFO = 0; nFO < nObj; nFO++)
			{
				cout << Pop[nP][nFO] << "\t";
			}
			cout << endl;
		}
	}

	//
	//	double extremeScenario(string extremeScenarioLP, double coefTotalCost, double coefWearTear, double coefMaxLoad)
	//	{
	//		IloEnv env;
	//		try
	//		{
	//
	//			IloCplex cplex(env);
	//			IloModel model(env);
	//			IloObjective obj;
	//			IloNumVarArray var(env);
	//			IloRangeArray rng(env);
	//			IloNumArray vals(env);
	//			IloBoolArray a(env);
	//
	//			string modelLPAdress = "./LP/" + extremeScenarioLP + ".lp";
	//			cplex.importModel(model, modelLPAdress.c_str(), obj, var, rng);
	//			cplex.extract(model);
	//			cplex.readMIPStart("teste.mst");
	//
	//			obj.setLinearCoef(var[0], coefTotalCost);
	//			obj.setLinearCoef(var[1], coefWearTear);
	//			obj.setLinearCoef(var[2], coefMaxLoad);
	//
	//			cplex.setParam(IloCplex::AdvInd, 1);
	//			cplex.setParam(IloCplex::AggCutLim, 2);
	//			if (!cplex.solve())
	//			{
	//				env.error() << "Failed to optimize LP" << endl;
	//				throw(-1);
	//			}
	//			cplex.getMIPStart(0, var, vals, a);
	//			//cplex.getValues(vals, var, 0);
	////			cout<<var<<endl;
	////			cout<<vals<<endl;
	//			cout << "objCost = " << vals[0] << endl;
	//			cout << "objWearTear = " << vals[1] << endl;
	//			cout << "objMaxLoad = " << vals[2] << endl;
	//
	//			//cplex.getValues(vals, var, 0);
	//
	//			cout << "OKKKK!!!" << endl;
	//			getchar();
	//
	//		} catch (IloException& e)
	//		{
	//			cerr << "Concert exception caught: " << e << endl;
	//		} catch (...)
	//		{
	//			cerr << "Unknown exception caught" << endl;
	//		}
	//	}

	vector<double> calcMeanDesv(vector<double> v)
	{
		double sum = accumulate(v.begin(), v.end(), 0.0);
		double mean = sum / v.size();

		double sq_sum = inner_product(v.begin(), v.end(), v.begin(), 0.0);
		double stdev = sqrt(sq_sum / v.size() - mean * mean);

		vector<double> result;
		result.push_back(sum);
		result.push_back(stdev);
		return result;
	}

	vector<double> readDifferentScenarios(vector<double> energySelling, vector<double> energyBuying, vector<vector<vector<bool> > > energyC, vector<vector<vector<bool> > > energyD, double foCost, double foMaxPeakLoad, double totalCDPayed, int extremeScenario, int nDiscrete, int nPEV)
	{

		vector<vector<double> > pevsDischargeRate;
		vector<vector<double> > pevsChargeRate;
		vector<double> pevsPower;
		pevsPower.push_back(60);
		pevsPower.push_back(30);
		pevsPower.push_back(60);

		File* filePev;
		string pevConfig = "./LP/differentScenarios/PEVsConfig";
		try
		{
			filePev = new File(pevConfig);
		} catch (FileNotFound& f)
		{
			cout << "Parameters File '" << pevConfig << "' not found" << endl;
			exit(1);
		}
		Scanner scannerPevsConfig(filePev);

		for (int pev = 0; pev < nPEV; pev++)
		{
			vector<double> dischargeRate;
			for (int c = 0; c < nDiscrete; c++)
				dischargeRate.push_back(scannerPevsConfig.nextDouble());

			pevsDischargeRate.push_back(dischargeRate);

//			for (int c = 0; c < nDiscrete; c++)
//				cout << pevsDischargeRate[pev][c] << "\t";
//			cout << endl;
//			getchar();

		}

		for (int pev = 0; pev < nPEV; pev++)
		{
			vector<double> chargeRate;
			for (int c = 0; c < nDiscrete; c++)
				chargeRate.push_back(scannerPevsConfig.nextDouble());

			pevsChargeRate.push_back(chargeRate);

//			for (int c = 0; c < nDiscrete; c++)
//				cout << pevsChargeRate[pev][c] << "\t";
//			cout << endl;
//			getchar();
		}

		double sharpeRatioVolatilityBest = 1;
		double sharpeRatioVolatilityWorst = 1;

		vector<vector<vector<double> > > scenariosData;
		vector<string> scenarios;
		scenarios.push_back("./LP/differentScenarios/worstCase");
		scenarios.push_back("./LP/differentScenarios/bestCase");
		for (int r = 1; r <= 20; r++)
		{
			stringstream ss;
			ss << "./LP/differentScenarios/randomCase" << r;
			scenarios.push_back(ss.str());
		}

		for (int r = 0; r < scenarios.size(); r++)
		{
			File* file;
			try
			{
				file = new File(scenarios[r]);
			} catch (FileNotFound& f)
			{
				cout << "Parameters File '" << scenarios[r] << "' not found" << endl;
				exit(1);
			}
			Scanner scanner(file);

			vector<vector<double> > scenarioCurrentData;
			for (int k = 0; k < 24; k++)
			{
				vector<double> values;
				double consum = scanner.nextDouble();
				double production = scanner.nextDouble();
				double priceBuy = scanner.nextDouble();
				double priceSell = scanner.nextDouble();
				double rate = scanner.nextDouble();
//				cout<<consum <<"\t"<<production<<"\t"<<priceBuy<<"\t"<<priceSell<<"\t"<<rate<<endl;
//				getchar();
				values.push_back(consum); //consumption
				values.push_back(production); //product
				values.push_back(priceBuy); //price
				values.push_back(priceSell); //price sell
				values.push_back(rate); //rate
				scenarioCurrentData.push_back(values);
			}

			scenariosData.push_back(scenarioCurrentData);

		}

		// ==================================================
		// EXTREME FO CALCULUS

		double newFoExtreme = 0;

//		vector<double> newEnergySelling(24, 0);
//		vector<double> newEnergyBuying(24, 0);
		for (int k = 0; k < 24; k++)
		{
			double consum = scenariosData[extremeScenario][k][0];
			double rg = scenariosData[extremeScenario][k][1];
			double priceBuy = scenariosData[extremeScenario][k][2];
			double priceSell = scenariosData[extremeScenario][k][3];
			double rate = scenariosData[extremeScenario][k][4];

			for (int pev = 0; pev < nPEV; pev++)
			{
//				for (int c = 0; c < nDiscrete; c++)
//					cout << energyC[k][pev][c] << "\t";
//				cout << endl;
//				for (int c = 0; c < nDiscrete; c++)
//					cout << energyD[k][pev][c] << "\t";
//				cout << endl;
//				getchar();

				for (int c = 0; c < nDiscrete; c++)
				{

					if (energyC[k][pev][c] == true)
					{
						rate += pevsChargeRate[pev][c] * pevsPower[pev] / 100;
					}

					if (energyD[k][pev][c] == true)
					{
						rate -= pevsDischargeRate[pev][c] * pevsPower[pev] / 100;
					}
				}

			}
//			cout << "old rate = " << energyBuying[k] - energySelling[k] << endl;
//			cout << "rate = " << rate << endl;
//			getchar();
			double newAmountBought;
			double newAmountSold;
			if (rate > 0)
				newAmountBought = rate;
			if (rate < 0)
				newAmountSold = rate;

			if (energySelling > energyBuying)
			{
				newFoExtreme += newAmountSold * priceSell * -1;
			}

			if (energyBuying > energySelling)
			{
				newFoExtreme += newAmountBought * priceBuy;
			}

		}

		newFoExtreme = newFoExtreme + totalCDPayed;

		double foExtremeScenario = 0;

		if (foCost > 0)
		{
			if (newFoExtreme > 0)
				foExtremeScenario = foCost / newFoExtreme;
			else
				foExtremeScenario = (abs(newFoExtreme) + foCost) / foCost;
		}
		else
		{
			if (newFoExtreme > 0)
				foExtremeScenario = abs(foCost) / (newFoExtreme + abs(foCost));
			else
				foExtremeScenario = abs(newFoExtreme) / abs(foCost);
		}

		// EXTREME FO CALCULUS END
		// ==================================================

		//==================================
		//SHARPE RATIO CALCULUSS

		vector<double> newTotalCosts(scenariosData.size(), 0);
		vector<double> newMaxLoads(scenariosData.size(), 0);
		for (int s = 0; s < scenariosData.size(); s++)
		{
			for (int k = 0; k < 24; k++)
			{
				double consum = scenariosData[s][k][0];
				double rg = scenariosData[s][k][1];
				double priceBuy = scenariosData[s][k][2];
				double priceSell = scenariosData[s][k][3];
				double rate = scenariosData[s][k][4];

				for (int pev = 0; pev < nPEV; pev++)
				{
					//				for (int c = 0; c < nDiscrete; c++)
					//					cout << energyC[k][pev][c] << "\t";
					//				cout << endl;
					//				for (int c = 0; c < nDiscrete; c++)
					//					cout << energyD[k][pev][c] << "\t";
					//				cout << endl;
					//				getchar();

					for (int c = 0; c < nDiscrete; c++)
					{

						if (energyC[k][pev][c] == true)
						{
							rate += pevsChargeRate[pev][c] * pevsPower[pev] / 100;
						}

						if (energyD[k][pev][c] == true)
						{
							rate -= pevsDischargeRate[pev][c] * pevsPower[pev] / 100;
						}
					}

				}
				//			cout << "old rate = " << energyBuying[k] - energySelling[k] << endl;
				//			cout << "rate = " << rate << endl;
				//			getchar();
				double newAmountBought;
				double newAmountSold;
				if (rate > 0)
					newAmountBought = rate;
				if (rate < 0)
					newAmountSold = rate;

				if (energySelling > energyBuying)
				{
					newFoExtreme += newAmountSold * priceSell * -1;
				}

				if (energyBuying > energySelling)
				{
					newFoExtreme += newAmountBought * priceBuy;
				}

				newTotalCosts[s] = newFoExtreme;

				if (abs(rate) > newMaxLoads[s])
					newMaxLoads[s] = abs(rate);

			}
		}

		//=========================

		vector<double> meanSTDSharpeTotalCost = calcMeanDesv(newTotalCosts);
		vector<double> meanSTDSharpeMaxLoad = calcMeanDesv(newMaxLoads);

//		cout<<meanSTDSharpeTotalCost[0]<<"\t"<<meanSTDSharpeTotalCost[1]<<endl;
//		cout<<meanSTDSharpeMaxLoad[0]<<"\t"<<meanSTDSharpeMaxLoad[1]<<endl;
//		getchar();
		double freeOfRiskCost;
		double freeOfRiskMaxLoad;
		if (extremeScenario == 1)
		{
			freeOfRiskCost = 94267;				//TODO
			freeOfRiskMaxLoad = 110;			//TODO %based on wear and tear
		}
		{
			freeOfRiskCost = -4239.6;				//TODO
			freeOfRiskMaxLoad = 70;				//TODO %based on wear and tear
		}

		double foSharpeRatioTotalCost = (freeOfRiskCost - foCost) / meanSTDSharpeTotalCost[1];
		double foSharpeRatioMaxPeakLoad = (freeOfRiskMaxLoad - foMaxPeakLoad) / meanSTDSharpeMaxLoad[1];

		vector<double> newFos;
		newFos.push_back(foExtremeScenario);
		newFos.push_back(foSharpeRatioTotalCost);
		newFos.push_back(foSharpeRatioMaxPeakLoad);

		return newFos;

	}

	vector<int> findLPNumberVariables(IloNumVarArray& var)
	{
		vector<int> variablesNumber;

//		cout << var << endl;
		//objCost[-inf..inf] , objWearTear[-inf..inf] , objMaxLoad[-inf..inf]
		// energySelling(1),energyBuying(1)
		// yCharge(C1,1,1), yCharge(C1,2,1), ..., yCharge(C3,Cycle,1), ..., yDischarge(C3,Cyle,1)
		// energySelling(..),energyBuying(..)
		// yCharge(....) ... yDischarge(...)
		// BaterryRate(C1,1), BaterryRate(C2,1), ...,  yBaterryRate(pev,Interval)
		// energySellingActive(1), energySellingActive(2), ..., energyBuyingActive(1), energyBuyingActive(2),...
		// totalChargingDischargingPayed
		//						for (int lpVar = 0; lpVar < var.getSize(); lpVar++)
		//						{
		//							string varName = var[lpVar].getName();
		//							cout << varName.c_str() << endl;
		//							int variableFound = varName.find("yCharge(C1");

		cout << "Finding LP number of variable..." << endl;
		int counterNumberCycles = 0;
		int countPEVS = 0;
		int countIntervals = 0;
		int variableFound = 0;
		string varName;
		while (variableFound == 0)
		{
			counterNumberCycles++;
			varName = var[5 + counterNumberCycles].getName();
			variableFound = varName.find("yCharge(C1");
		}
		variableFound = 0;
		while (variableFound == 0)
		{
			countPEVS++;
			varName = var[5 + countPEVS].getName();
			variableFound = varName.find("yCharge");
		}

		for (int lpVar = 5; lpVar < var.getSize(); lpVar++)
		{
			varName = var[lpVar].getName();
			variableFound = varName.find("yBaterryRate(C1,2");

			while (variableFound == 0)
			{
				countIntervals++;
				varName = var[lpVar + countIntervals].getName();
				variableFound = varName.find("yBaterryRate(C1");
			}

			if (countIntervals > 0)
			{
				// first constraints was not counted, started cycle 2
				countIntervals = countIntervals + 1;
				lpVar = var.getSize();
			}
		}
		countPEVS /= counterNumberCycles;
//		cout << counterNumberCycles << endl;
//		cout << countPEVS << endl;
//		cout << countIntervals << endl;
//		getchar();
		variablesNumber.push_back(counterNumberCycles);
		variablesNumber.push_back(countPEVS);
		variablesNumber.push_back(countIntervals);

		return variablesNumber;
	}

	void exec(string filename, bool mipStart, int nIntervalsCoef, int tLim)
	{

		cout << "Exec Math Model Smart Energy Storage" << endl;

		int nObj = 3;
		cout << "Number of objectives functions: " << nObj << endl;
		UnionNDSets2 uND(nObj);

		vector<string> vInputModel;
//		vInputModel.push_back("bWCM");
//		vInputModel.push_back("arqRealData24Worst");
//		vInputModel.push_back("arqRealData24Best");
		vInputModel.push_back(filename.c_str());

		bool playWithBestMIPStartSolution = mipStart;
		//forcing playWithBestMIPStartSolution
//		playWithBestMIPStartSolution = true;

		vector<int> vNMaxOpt;
		//TODO verificar erro quando valor eh 5 -
		// Updated: March 2016 -- seams to be fixed adding one element to vector of coeficients
		vNMaxOpt.push_back(nIntervalsCoef);

		vector<int> vTLim;
		vTLim.push_back(tLim);

		int nIntervals, nDiscrete, nPEV, nVariablesPerInterval, nTotal;
		int nObjMILP = 3;

		for (int iM = 0; iM < vInputModel.size(); iM++)
			for (int nM = 0; nM < vNMaxOpt.size(); nM++)
				for (int tL = 0; tL < vTLim.size(); tL++)
				{
					int nExpBatches = vNMaxOpt[nM];

					vector<double> vCoefTotalCost(nExpBatches + 1);
					vector<double> vCoefMaxLoad(nExpBatches + 1);
					vector<double> vCoefWearTear(nExpBatches + 1);

					for (int n = 0; n <= nExpBatches; n++)
					{
						double coef = n * 1.00 / nExpBatches * 1.00;
						vCoefTotalCost[n] = coef;
						vCoefMaxLoad[n] = coef;
						vCoefWearTear[n] = coef;
//						cout << coef << endl;
					}

					vector<TupleMILPCoef> vCoef;
					for (int n1 = 0; n1 <= nExpBatches; n1++)
						for (int n2 = 0; n2 <= nExpBatches; n2++)
							for (int n3 = 0; n3 <= nExpBatches; n3++)
							{

								double coefTotalCost = vCoefTotalCost[n1];
								double coefWearTear = vCoefWearTear[n2];
								double coefMaxLoad = vCoefMaxLoad[n3];
								if ((coefTotalCost == 0) && (coefWearTear == 0) && (coefMaxLoad == 0))
								{
									//In the current MILP model, maxLoad Obj should always be minimized
									TupleMILPCoef milpCoef(0, 0, 0.01);
									vCoef.push_back(milpCoef);
								}
								else
								{

									TupleMILPCoef milpCoef(coefTotalCost, coefWearTear, coefMaxLoad);
									vCoef.push_back(milpCoef);
								}
							}
//					getchar();

					vector<vector<double> > Pop;
					vector<vector<double> > PopGridRate;
					IloEnv env;
					try
					{
						Timer tTotal;
						IloCplex cplex(env);
						IloModel model(env);
						IloObjective obj;
						IloNumVarArray var(env);
						IloRangeArray rng(env);

						string modelLPAdress = "./LP/" + vInputModel[iM] + ".lp";
						cout << "solving: " << modelLPAdress << endl;

						cplex.importModel(model, modelLPAdress.c_str(), obj, var, rng);

						// ========= Finding LP number of variable ==============
						vector<int> numberVariables = findLPNumberVariables(var);
						nDiscrete = numberVariables[0];
						nPEV = numberVariables[1];
						nIntervals = numberVariables[2];
						nVariablesPerInterval = (2 * nDiscrete * nPEV) + 2;
						nTotal = nIntervals * nVariablesPerInterval + nObjMILP;

						cout << "number of Discharge and Charges cycles = " << nDiscrete << endl;
						cout << "number of nPEVs = " << nPEV << endl;
						cout << "number of nIntervals = " << nIntervals << endl;
//						getchar();
						// ========= Finding LP number of variable ==============

						int nMILPProblems = vCoef.size();
						cout << "nMILPProblems = " << nMILPProblems << endl;
//						getchar();
						bool milpHasBeenSaved = false;
						vector<MIPStartSolution> poolMIPStart;
						for (int milpProblems = 0; milpProblems <= nMILPProblems; milpProblems++)
						{
							double coefTotalCost = vCoef[milpProblems].coefTotalCost;
							double coefWearTear = vCoef[milpProblems].coefWearTear;
							double coefMaxLoad = vCoef[milpProblems].coefMaxLoad;

//										if (coefTotalCost < 0.15)
//											coefTotalCost = 0.001;
//							if (coefMaxLoad < 0.01)
//								coefMaxLoad = 0.01;

							cout << "=====================================\n";
							cout << "Creating MILP model with: \n";
							cout << "coefTotalCost: " << coefTotalCost;
							cout << "\tcoefWearTear: " << coefWearTear;
							cout << "\tcoefMaxLoad: " << coefMaxLoad << endl << endl;
//										getchar();
							//cout << obj << endl;
							//	getchar();
							obj.setLinearCoef(var[0], coefTotalCost);
							obj.setLinearCoef(var[1], coefWearTear);
							obj.setLinearCoef(var[2], coefMaxLoad);
							//								cout << obj << endl;
							//								getchar();

							/*

							 model.remove(obj);

							 getchar();
							 IloExpr newObjFunc(env);
							 //newObjFunc += IloNumVar(env, 0.0, IloInfinity, ILOFLOAT, "objFuncTotalDist" );
							 newObjFunc += obj.getExpr();
							 cout << newObjFunc << endl;

							 // obj.setExpr(newObjFunc);

							 //obj = IloMinimize(env, newObjFunc,"newObjFunc");
							 cout << obj << endl;

							 //model.add(obj);
							 // cplex.importModel(model, modelAdress.c_str(), obj, var, rng);
							 getchar();
							 */

							cplex.extract(model);
							cplex.setParam(cplex.TiLim, vTLim[tL]);

//============== PLAYING WITH STARTING MIP==============///
//============================================================///
//							if (milpHasBeenSaved)
//								cplex.readMIPStarts("./poolMIPStarts.mst");

//							cplex.readMIPStart("./LP/sol.mst");
//							cplex.readMIPStarts("./LP/allSol.mst");
//							cplex.readMIPStarts("./LP/sol.mst");
//							cplex.readMIPStarts("./LP/sol2.mst");

//							IloNumArray valsMIP(env);
//							cplex.getValues(valsMIP, var, 0);

//							IloNumVarArray startVar(env);
//							IloNumArray startVal(env);
//							for (int nV = 3; nV < nTotal; nV += nVariablesPerInterval)
//							{
//								for (int pev = 0; pev < nPEV; pev++)
//									for (int c = 0; c < nDiscrete; c++)
//									{
//										//cout<<nV + 1 + pevC + pev * nDiscrete<<endl;
//										startVar.add(var[nV + 1 + c + 1 + pev * nDiscrete]);
//										startVal.add(rg.rand(2));
//
//									}
//
//								for (int pev = 0; pev < nPEV; pev++)
//									for (int c = 0; c < nDiscrete; c++)
//									{
//										startVar.add(var[nV + 1 + c + 1 + pev * nDiscrete]);
//										startVal.add(rg.rand(2));
//									}
//							}

//							cplex.addMIPStart(startVar, startVal);
//							cout << startVal << endl;
//							cout << startVal << endl;

//							getchar();

							if (playWithBestMIPStartSolution)
							{
								int totalNMIPStartSolutions = poolMIPStart.size();
								if (totalNMIPStartSolutions > 0)
								{
									cout << "Pool of MIP start solution has: " << totalNMIPStartSolutions << " possibilities" << endl;

									double bestFO = 100000000000000;
									int bestMIPStartIndex = -1;
									//Selecting best MIP start for the current weights
									for (int mipS = 0; mipS < totalNMIPStartSolutions; mipS++)
									{

										double mipObj = poolMIPStart[mipS].objCost;
										double mipWearTear = poolMIPStart[mipS].objWearTear;
										double mipMaxLoad = poolMIPStart[mipS].objMaxPeakLoad;

										double currentMILPObj = coefTotalCost * mipObj + mipWearTear * coefWearTear + coefMaxLoad * mipMaxLoad;
//										cout << "MIP start obj values:" << endl;
										//									cout << "objCost = " << mipObj << endl;
//									cout << "objWearTear = " << mipWearTear << endl;
//									cout << "objMaxLoad = " << mipMaxLoad << endl;
//									cout << "MILP obj with current coef  = " << currentMILPObj << endl;

										if (currentMILPObj < bestFO)
										{
											bestFO = currentMILPObj;
											bestMIPStartIndex = mipS;
//										   cout << "bestFO: " << bestFO << "\t index:" << bestMIPStartIndex << endl;
										}
//									getchar();
									}

									cout << "best MIPStart solution is: " << bestFO << "\t index:" << bestMIPStartIndex << endl << endl;

									//=======================

									IloNumVarArray startVar(env);
									IloNumArray startVal(env);
									int mipStartIntervalIDX = 0;
									MIPStartSolution bestMIPStart = poolMIPStart[bestMIPStartIndex];
									for (int nV = 3; nV < nTotal; nV += nVariablesPerInterval)
									{
										for (int pev = 0; pev < nPEV; pev++)
											for (int c = 0; c < nDiscrete; c++)
											{
												startVar.add(var[nV + 1 + c + 1 + pev * nDiscrete]);
												startVal.add(bestMIPStart.pevsCharge[mipStartIntervalIDX][pev][c]);
											}

										for (int pev = 0; pev < nPEV; pev++)
											for (int c = 0; c < nDiscrete; c++)
											{
												startVar.add(var[nV + 1 + nPEV * nDiscrete + c + 1 + pev * nDiscrete]);
												startVal.add(bestMIPStart.pevsDischarge[mipStartIntervalIDX][pev][c]);
											}

										mipStartIntervalIDX++;
									}
									cplex.addMIPStart(startVar, startVal);

								}
							}

//==============FINISH PLAYING WITH STARTING MIP==============///
//============================================================///

							//cplex.setParam(cplex.MIPEmphasis, cplex.MIPEmphasisHiddenFeas);
							IloBool solveBool = cplex.solve();
							int nCplexPoolOfSolutions = cplex.getSolnPoolNsolns();

							if (!solveBool)
							{
//								env.error() << "Failed to optimize LP" << endl;
//								throw(-1);
								cout << "=====================================\n";
								cout << "Any solution was found! The following parameters were used: \n";
								cout << "coefTotalCost: " << coefTotalCost;
								cout << "\t coefWearTear: " << coefWearTear;
								cout << "\t coefMaxLoad: " << coefMaxLoad << "\n";
								cout << "\t cplex.TiLim: " << vTLim[tL] << "\n";
								cout << "=====================================\n";
							}
							else
							{
								cout << "=====================================\n";
								cout << "Problem SOLVED!! \n";
								cout << nCplexPoolOfSolutions << " solutions were obtained. \n";
								cout << "=====================================\n\n";
							}
							//	getchar();

							//getchar();
							IloNumArray vals(env);
							if (nCplexPoolOfSolutions > 0)
							{
								cplex.writeMIPStarts("temp.mst", 0, nCplexPoolOfSolutions);
								cplex.writeSolutions("tempSol.sol");
							}

//							if (nCplexPoolOfSolutions > 0)
//							{
//
//								if (!milpHasBeenSaved) // This is the first time
//								{
//									cout << "writeMIPS " << cplex.getNMIPStarts() << " starts solutions for the first time" << endl;
//									cplex.writeMIPStarts("poolMIPStarts.mst", 0, nCplexPoolOfSolutions);
//									totalMIPStartSolutions += nCplexPoolOfSolutions;
//
//									std::ifstream f("poolMIPStarts.mst");
//									f.seekg(0, f.end);
//									int length = f.tellg();
//									cout << "length = " << length << endl;
//									getchar();
//
//								}
//								else
//								{
//									cout << "adding solution to the main MIP solutions files" << endl;
//									cplex.writeMIPStarts("temp.mst", 0, nCplexPoolOfSolutions);
//									totalMIPStartSolutions += nCplexPoolOfSolutions;
//
//								}
//
//								cout << "cplex.getNMIPStarts2(): " << cplex.getNMIPStarts() << endl;
//								cout << "totalMIPStartSolutions: " << totalMIPStartSolutions << endl;
//
//								getchar();
//
//								milpHasBeenSaved = true;
//							}

							if (nCplexPoolOfSolutions > 0)
								for (int nS = 0; nS < nCplexPoolOfSolutions; nS++)
								{
									vector<double> energySelling;
									vector<double> energyBuying;
									vector<vector<vector<bool> > > energyCharging;
									vector<vector<vector<bool> > > energyDischarging;
									vector<double> gridRate;
									vector<vector<double> > batteryRate;

									cout << "=====================================\n";
									cout << "Extracting solution: " << nS + 1 << "/" << nCplexPoolOfSolutions << endl;

									//objCost[-inf..inf] , objWearTear[-inf..inf] , objMaxLoad[-inf..inf]
									// energySelling(1),energyBuying(1)
									// yCharge(C1,1,1), yCharge(C1,2,1), ..., yCharge(C3,Cycle,1), ..., yDischarge(C3,Cyle,1)
									// energySelling(..),energyBuying(..)
									// yCharge(....) ... yDischarge(...)
									// BaterryRate(C1,1), BaterryRate(C2,1), ...,  yBaterryRate(pev,Interval)
									// energySellingActive(1), energySellingActive(2), ..., energyBuyingActive(1), energyBuyingActive(2),...
									// totalChargingDischargingPayed

									cplex.getValues(vals, var, nS);
//											cout << var << endl;
//											getchar();
									double objCost = vals[0];
									double objWearTear = vals[1];
									double objMaxPeakLoad = vals[2];

									cout << "objCost = " << objCost << endl;
									cout << "objWearTear = " << objWearTear << endl;
									cout << "objMaxLoad = " << objMaxPeakLoad << endl;

									double totalChargingDischargingPayed = vals[vals.getSize() - 1];
									cout << "totalChargingDischargingPayed = " << totalChargingDischargingPayed << endl;

									//								cplex.writeSolution("teste.mst", nS);
//								cout<<nMILPProblems<<endl;
//								getchar();
//								cplex.writeMIPStarts("solMIPStart.mst", nS);
//											extremeScenario(vInputModel[1], coefTotalCost, coefWearTear, coefMaxLoad);
//											getchar();
									// energySell + energyBuying + nDiscrete*yCharge + nDiscrete*yDischarge

									for (int nV = 3; nV < nTotal; nV += nVariablesPerInterval)
									{
										double energySold = vals[nV];
										double energyBought = vals[nV + 1];
										energySelling.push_back(energySold);
										energyBuying.push_back(energyBought);

										if (energyBought > energySold)
											gridRate.push_back(energyBought);
										else
											gridRate.push_back(energySold * -1);

										// =======================================================
										//Checks if energy is being sold and bought at the same time
										if ((energySold > 0.00001) && (energyBought > 0.00001))
										{
											cout << "BUG!" << endl;
											cout << "energyBought = " << energyBought << endl;
											cout << "energySold = " << energySold << endl;
//											cout << var << endl;
//											cout << vals << endl;
											cout << "BUG! energy being bought" << endl;
											getchar();
										}
										// =======================================================

										vector<vector<bool> > pevsCharges;

										for (int pev = 0; pev < nPEV; pev++)
										{
											vector<bool> pevCharge;
											for (int c = 0; c < nDiscrete; c++)
											{
												//cout<<nV + 1 + pevC + pev * nDiscrete<<endl;
												double boolCharge = vals[nV + 1 + c + 1 + pev * nDiscrete];
												if (boolCharge > 0.9)
													pevCharge.push_back(true);
												else
													pevCharge.push_back(false);
											}

											pevsCharges.push_back(pevCharge);

//													for (int c = 0; c < nDiscrete; c++)
//														cout << pevsCharges[pev][c] << "\t";
//													cout << endl;
//													getchar();

										}

										energyCharging.push_back(pevsCharges);

										vector<vector<bool> > pevsDischarge;

										for (int pev = 0; pev < nPEV; pev++)
										{
											vector<bool> pevDischargeCharge;
											for (int c = 0; c < nDiscrete; c++)
											{
												double boolDischarge = vals[nV + 1 + nPEV * nDiscrete + c + 1 + pev * nDiscrete];
												//cout<<nV + 1 +nPEV*nDiscrete + pevC + pev * nDiscrete<<endl;
												if (boolDischarge > 0.9)
													pevDischargeCharge.push_back(true);
												else
													pevDischargeCharge.push_back(false);
											}
											pevsDischarge.push_back(pevDischargeCharge);

//													for (int c = 0; c < nDiscrete; c++)
//														cout << pevsDischarge[pev][c] << "\t";
//													cout << endl;
//													getchar();
										}
										energyDischarging.push_back(pevsDischarge);

									}

									for (int pev = 0; pev < nPEV; pev++)
									{
										vector<double> pevBatteryRate;
										//First interval
										pevBatteryRate.push_back(vals[nTotal + pev]);

										int beginBatteryRateIndex = nTotal + pev * (nIntervals - 1) + nPEV;
										for (int nV = beginBatteryRateIndex; nV < beginBatteryRateIndex + (nIntervals - 1); nV++)
										{
											pevBatteryRate.push_back(vals[nV]);
										}
										batteryRate.push_back(pevBatteryRate);
									}

//								for (int it = 0; it < nIntervals; it++)
//								{
//									cout << "energySelling[" << it << "] = " << energySelling[it] << endl;
//									cout << "energyBuying[" << it << "] = " << energyBuying[it] << endl;
//									cout << "gridRate[" << it << "] = " << gridRate[it] << endl;
//									for (int pev = 0; pev < nPEV; pev++)
//										cout << "batteryRatePEV" << pev + 1 << "[" << it << "] = " << batteryRate[pev][it] << endl;
//								}
//								getchar();

									int exScenario = 1;
									if (iM == 1)
										int exScenario = 0;

//									vector<double> newFo = readDifferentScenarios(energySelling, energyBuying, energyCharging, energyDischarging, objCost, foMaxPeakLoad, totalChargingDischargingPayed, exScenario, nDiscrete, nPEV);

									vector<double> fo;
									fo.push_back(objCost); //total cost
									fo.push_back(objWearTear); // wear and tear
									fo.push_back(objMaxPeakLoad); // max load
//									fo.push_back(newFo[0]); //extreme
//									fo.push_back(newFo[1]); // sharpe ratio total cost
//									fo.push_back(newFo[2]); //sharpe ratio max load
//									fo.push_back(0); //extreme
//									fo.push_back(0); // sharpe ratio total cost
//									fo.push_back(0); //sharpe ratio max load
									Pop.push_back(fo);
									PopGridRate.push_back(gridRate);
//									Population<RepMILPStoragePlanning> p_a;
//									bool added = addSolution(pa_a, s);

									MIPStartSolution mipStartSol(energyDischarging, energyCharging, objCost, objMaxPeakLoad, objWearTear);
									poolMIPStart.push_back(mipStartSol);
//									cout<< s.getR().pevsCharge  <<endl;

									cout << "Solution: " << nS + 1 << "/" << nCplexPoolOfSolutions << " has been extracted with success and added to the population" << endl;
									cout << "=====================================\n";

								}

							cout << "=====================================\n\n";

						}
						cout << "total time spent: " << tTotal.now() << endl;
						cout << "Printing obtained population of solutions with size :" << Pop.size() << endl;
						printVectorOfVector(Pop);
						vector<vector<double> > paretoSET = uND.createParetoSet(Pop);
						pair<vector<vector<double> >, vector<vector<double> > > paretoPopRate = uND.createParetoSetSavingRate(Pop, PopGridRate);

						double nParetoInd = paretoSET.size();

						cout << "Printing Pareto Front of size:" << paretoSET.size() << endl;
						printVectorOfVector(paretoSET);
						vector<vector<vector<double> > > vParetoSet;
						vParetoSet.push_back(paretoSET);

						stringstream ss;
						if (mipStart)
						{
							ss << "./ResultadosFronteiras/" << vInputModel[iM] << "NExec" << pow(vNMaxOpt[nM], 3) << "TLim" << vTLim[tL] << "-bestMIPStart";
						}
						else
						{
							ss << "./ResultadosFronteiras/" << vInputModel[iM] << "NExec" << pow(vNMaxOpt[nM], 3) << "TLim" << vTLim[tL]; // << "-bestMIPStart";
						}

						FILE* fFronteiraPareto = fopen(ss.str().c_str(), "w");
						for (int nS = 0; nS < nParetoInd; nS++)
						{
							for (int nE = 0; nE < nObj; nE++)
							{
								fprintf(fFronteiraPareto, "%.5f \t ", paretoSET[nS][nE]);
							}
							fprintf(fFronteiraPareto, "\n");
						}
						fprintf(fFronteiraPareto, "%.5f \n ", tTotal.now());

						fclose(fFronteiraPareto);

//						stringstream ssRate;
//						ssRate << "./ResultadosFronteiras/ResultsREM2016" << vInputModel[iM] << "NExec" << pow(vNMaxOpt[nM], 3) << "TLim" << vTLim[tL] << "SolutionsRate";
//
//						FILE* fFronteiraParetoRate = fopen(ssRate.str().c_str(), "w");
//						for (int nS = 0; nS < nParetoInd; nS++)
//						{
//							for (int nE = 0; nE < nObj; nE++)
//							{
//								fprintf(fFronteiraParetoRate, "%.5f \t ", paretoPopRate.first[nS][nE]);
//							}
//							fprintf(fFronteiraParetoRate, "\n");
//							for (int i = 0; i < nIntervals; i++)
//							{
//								fprintf(fFronteiraParetoRate, "%.5f \t ", paretoPopRate.second[nS][i]);
//							}
//							fprintf(fFronteiraParetoRate, "\n \n");
//						}
//						fclose(fFronteiraParetoRate);

						//getchar();

						//getchar();
						//getchar();

						/*
						 env.out() << "Solution status = " << cplex.getStatus() << endl;
						 env.out() << "Solution value  = " << cplex.getObjValue() << endl;
						 cout << cplex.getBestObjValue() << endl;
						 //cout << cplex.getNbinVars() << endl;

						 //env.out() << "Solution vector = " << vals << endl;
						 for (int i = 0; i <= 40; i++)
						 {
						 for (int j = 0; j <= 40; j++)
						 {
						 int index = i * 41 + j;
						 //cout << var[var.getSize() - 1 - index] << " = " << vals[vals.getSize() - 1 - index] << endl;
						 if (vals[var.getSize() - 1 - index] == 1)
						 {
						 //cout << var[var.getSize() - 1 - index] << " = " << vals[vals.getSize() - 1 - index] << "\t";
						 cout << i << "\t" << j << endl;
						 //getchar();
						 }

						 }
						 }
						 */

						//cout << vals[vals.getSize()] << endl;
						try
						{ // basis may not exist
							IloCplex::BasisStatusArray cstat(env);
							cplex.getBasisStatuses(cstat, var);
							env.out() << "Basis statuses  = " << cstat << endl;
						} catch (...)
						{
						}

						//env.out() << "Maximum bound violation = " << cplex.getQuality(IloCplex::MaxPrimalInfeas) << endl;
					} catch (IloException& e)
					{
						cerr << "Concert exception caught: " << e << endl;
					} catch (...)
					{
						cerr << "Unknown exception caught" << endl;
					}

					env.end();
				}

	}

// 2

}
;
}
#endif /* OPTIMALLINEARREGRESSION_HPP_ */
