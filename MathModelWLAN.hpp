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
#include <vector>
#include <numeric>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include "OptFrame/Util/MultiObjectiveMetrics2.hpp"
#include "OptFrame/RandGen.hpp"

ILOSTLBEGIN

using namespace std;
using namespace optframe;

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
			freeOfRiskMaxLoad = 110;				//TODO %based on wear and tear
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


	void exec()
	{

		cout << "Exec Math Model Smart Energy Storage" << endl;

		int nObj = 6;
		UnionNDSets2 uND(nObj);

		vector<string> vInputModel;
		vInputModel.push_back("arqRealData24Worst");
		vInputModel.push_back("arqRealData24Best");

		vector<int> vNMaxOpt;
		vNMaxOpt.push_back(20); //TODO verificar erro quando valor eh 5
		vector<int> vTLim;
		vTLim.push_back(60);

		int nIntervals = 24;
		int nDiscrete = 11;
		int nPEV = 3;

		for (int iM = 0; iM < vInputModel.size(); iM++)
			for (int nM = 0; nM < vNMaxOpt.size(); nM++)
				for (int tL = 0; tL < vTLim.size(); tL++)
				{
					int nExpBatches = vNMaxOpt[nM];

					vector<double> vCoefTotalCost(nExpBatches);
					vector<double> vCoefMaxLoad(nExpBatches);
					vector<double> vCoefWearTear(nExpBatches);
					for (int n = 0; n <= nExpBatches; n++)
					{
						double coef = n * 1.00 / nExpBatches * 1.00;
						vCoefTotalCost[n] = coef;
						vCoefMaxLoad[n] = coef;
						vCoefWearTear[n] = coef;
					}

					vector<vector<double> > Pop;
					vector<vector<double> > PopGridRate;
					IloEnv env;
					try
					{

						IloCplex cplex(env);
						IloModel model(env);
						IloObjective obj;
						IloNumVarArray var(env);
						IloRangeArray rng(env);

						string modelLPAdress = "./LP/" + vInputModel[iM] + ".lp";

						cplex.importModel(model, modelLPAdress.c_str(), obj, var, rng);

						for (int n1 = 0; n1 <= nExpBatches; n1++)
							for (int n2 = 0; n2 <= nExpBatches; n2++)
								for (int n3 = 0; n3 <= nExpBatches; n3++)
								{
									if ((n1 == 0) && (n2 == 0) && (n3 == 0))
									{
										//NOTHING TO DO
									}
									else
									{
										double coefTotalCost = vCoefTotalCost[n1];
										double coefWearTear = vCoefWearTear[n2];
										double coefMaxLoad = vCoefMaxLoad[n3];

//										if (coefTotalCost < 0.15)
//											coefTotalCost = 0.001;
//
										if (coefMaxLoad < 0.1)
											coefMaxLoad = 0.01;

										cout << "coefTotalCost" << coefTotalCost << endl;
										cout << "coefWearTear" << coefWearTear << endl;
										cout << "coefMaxLoad" << coefMaxLoad << endl;
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
										//cplex.setParam(cplex.MIPEmphasis, cplex.MIPEmphasisHiddenFeas);
										if (!cplex.solve())
										{
											env.error() << "Failed to optimize LP" << endl;
											throw(-1);
										}
										cout << "Problem SOLVED!!" << endl;
										//getchar();
										IloNumArray vals(env);

										for (int nS = 0; nS < cplex.getSolnPoolNsolns(); nS++)
										{
											vector<double> energySelling;
											vector<double> energyBuying;
											vector<vector<vector<bool> > > energyCharging;
											vector<vector<vector<bool> > > energyDischarging;
											vector<double> gridRate;
											vector<vector<double> > batteryRate;

											cout << "Solution: " << nS + 1 << endl;

											cplex.getValues(vals, var, nS);
//											cout << var << endl;
//											getchar();
											double objCost = vals[0];
											double foWearTear = vals[1];
											double foMaxPeakLoad = vals[2];

//											cout << "objCost = " << objCost << endl;
//											cout << "objWearTear = " << foWearTear << endl;
//											cout << "objMaxLoad = " << foMaxPeakLoad << endl;
//											getchar();

											double totalChargingDischargingPayed = vals[vals.getSize() - 1];
											cout << "totalChargingDischargingPayed = " << totalChargingDischargingPayed << endl;

											cplex.writeSolution("teste.mst", nS);
//											cplex.writeMIPStarts("teste.mst", nS);
//											extremeScenario(vInputModel[1], coefTotalCost, coefWearTear, coefMaxLoad);
//											getchar();
											// energySell + energyBuying + nDiscrete*yCharge + nDiscrete*yDischarge
											int nVariablesPerInterval = (2 * nDiscrete * nPEV) + 2;

											int nTotal = nIntervals * nVariablesPerInterval + nObj;

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

												if ((energySold > 0.00001) && (energyBought > 0.00001))
												{
													cout << "BUG!" << endl;
													cout << "energyBought = " << energyBought << endl;
													cout << "energySold = " << energySold << endl;
													cout << var << endl;
													cout << vals << endl;
													getchar();
												}

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

//											for (int it = 0; it < nIntervals; it++)
//											{
//												cout << "energySelling[" << it << "] = " << energySelling[it] << endl;
//												cout << "energyBuying[" << it << "] = " << energyBuying[it] << endl;
//												cout << "gridRate[" << it << "] = " << gridRate[it] << endl;
//												cout << "batteryRatePEV1[" << it << "] = " << batteryRate[0][it] << endl;
//												cout << "batteryRatePEV2[" << it << "] = " << batteryRate[1][it] << endl;
//												cout << "batteryRatePEV3[" << it << "] = " << batteryRate[2][it] << endl;
//											}
//											getchar();

											int exScenario = 1;
											if (iM == 1)
												int exScenario = 0;
											vector<double> newFo = readDifferentScenarios(energySelling, energyBuying, energyCharging, energyDischarging, objCost, foMaxPeakLoad, totalChargingDischargingPayed, exScenario, nDiscrete, nPEV);

											vector<double> fo;
											fo.push_back(vals[0]); //total cost
											fo.push_back(vals[1]); // wear and tear
											fo.push_back(vals[2]); // max load
											fo.push_back(newFo[0]); //extreme
											fo.push_back(newFo[1]); // sharpe ratio total cost
											fo.push_back(newFo[2]); //sharpe ratio max load
											Pop.push_back(fo);
											PopGridRate.push_back(gridRate);
										}

									}

								}

						printVectorPareto(Pop, nObj);
						vector<vector<double> > paretoSET = uND.createParetoSet(Pop);
						pair<vector<vector<double> >, vector<vector<double> > > paretoPopRate = uND.createParetoSetSavingRate(Pop, PopGridRate);

						double nParetoInd = paretoSET.size();

						cout << "Pareto front" << endl;
						printVectorPareto(paretoSET, nObj);
						vector<vector<vector<double> > > vParetoSet;
						vParetoSet.push_back(paretoSET);

						stringstream ss;
						ss << "./ResultadosFronteiras/ParetoFrontInput" << vInputModel[iM] << "NExec" << pow(vNMaxOpt[nM], 3) << "TLim" << vTLim[tL];

						FILE* fFronteiraPareto = fopen(ss.str().c_str(), "w");
						for (int nS = 0; nS < nParetoInd; nS++)
						{
							for (int nE = 0; nE < nObj; nE++)
							{
								fprintf(fFronteiraPareto, "%.5f \t ", paretoSET[nS][nE]);
							}
							fprintf(fFronteiraPareto, "\n");
						}
						fclose(fFronteiraPareto);

						stringstream ssRate;
						ssRate << "./ResultadosFronteiras/ParetoFrontInput" << vInputModel[iM] << "NExec" << pow(vNMaxOpt[nM], 3) << "TLim" << vTLim[tL] << "SolutionsRate";

						FILE* fFronteiraParetoRate = fopen(ssRate.str().c_str(), "w");
						for (int nS = 0; nS < nParetoInd; nS++)
						{
							for (int nE = 0; nE < nObj; nE++)
							{
								fprintf(fFronteiraParetoRate, "%.5f \t ", paretoPopRate.first[nS][nE]);
							}
							fprintf(fFronteiraParetoRate, "\n");
							for (int i = 0; i < nIntervals; i++)
							{
								fprintf(fFronteiraParetoRate, "%.5f \t ", paretoPopRate.second[nS][i]);
							}
							fprintf(fFronteiraParetoRate, "\n \n");
						}
						fclose(fFronteiraParetoRate);

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

};

#endif /* OPTIMALLINEARREGRESSION_HPP_ */
