#ifndef READPARETOSETS_HPP_
#define READPARETOSETS_HPP_

#include "OptFrame/Util/MultiObjectiveMetrics2.hpp"
#include "OptFrame/RandGen.hpp"

#include <unistd.h>

using namespace std;
using namespace optframe;

class readParetoSets
{
public:

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

	readParetoSets()
	{

	}

	~readParetoSets()
	{

	}

	double hipervolume(vector<vector<double> > v, vector<double> objReferences)
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
		ss << "./hv\t -r \"" << objReferences[0] << " " << objReferences[1] << " " << objReferences[2] << "\" \t" << tempFile.c_str();
		string hvValueString = execCommand(ss.str().c_str());
		double hvValue = atof(hvValueString.c_str());
		return hvValue;
	}

	void exec(string filename, bool mipStartNotUsed, int nIntervalsCoefNotUsed, int tLimNotUsed)
	{
		cout << "REM -- Checking sets of non-dominated solution! " << endl;
		int nObj = 3;
		cout << "Number of objectives functions: " << nObj << endl;
		UnionNDSets2 uND(nObj);

		vector<string> vInputModel;
		vInputModel.push_back(filename.c_str());

		vector<int> vNMaxOpt;
		vNMaxOpt.push_back(2);
		vNMaxOpt.push_back(3);
		vNMaxOpt.push_back(5);

		vector<int> vTLim;
		vTLim.push_back(5);
		vTLim.push_back(10);
		vTLim.push_back(20);

		vector<vector<vector<double> > > vParetoSet;
		vector<string> vParetoNames;
		vector<vector<double> > paretoSetRef;

		vector<double> maxEval(nObj, -1000000000);
		vector<double> minEval(nObj, 1000000000);

		for (int iM = 0; iM < vInputModel.size(); iM++)
			for (int nM = 0; nM < vNMaxOpt.size(); nM++)
				for (int tL = 0; tL < vTLim.size(); tL++)
					for (int mipStart = 0; mipStart <= 1; mipStart++)
					{
						stringstream ss;
						if (mipStart)
						{
							ss << "./ResultadosFronteiras/" << vInputModel[iM] << "NExec" << pow(vNMaxOpt[nM], 3) << "TLim" << vTLim[tL] << "-bestMIPStart";
						}
						else
						{
							ss << "./ResultadosFronteiras/" << vInputModel[iM] << "NExec" << pow(vNMaxOpt[nM], 3) << "TLim" << vTLim[tL]; // << "-bestMIPStart";
						}

						cout << "trying to read pareto set of model = " << vInputModel[iM] << endl;
						cout << "max = " << vNMaxOpt[nM] << endl;
						cout << "time limit = " << vTLim[tL] << endl;
						cout << "bestMIPStart: " << mipStart << endl;
						cout << "filename is:" << ss.str() << endl;

						bool paretoExist = false;
						File* file;
						try
						{
							file = new File(ss.str());
							paretoExist = true;
							vParetoNames.push_back(ss.str());
						} catch (FileNotFound& f)
						{
							cout << "File '" << ss.str() << "' not found" << endl;
							cout << "any pareto was found for this this configuration!" << endl << endl;
						}

						if (paretoExist)
						{
							Scanner scanner(new File(ss.str()));
							vector<vector<double> > paretoSet;

							while (scanner.hasNext())
							{
								vector<double> fo;

								for (int obj = 0; obj < nObj; obj++)
								{
									double objValue = scanner.nextDouble();
									fo.push_back(objValue);

									if (objValue > maxEval[obj])
										maxEval[obj] = objValue;
									if (objValue < minEval[obj])
										minEval[obj] = objValue;

									//	Gambiarra porque o tempo foi reportado no final do arquivo
									if (!scanner.hasNext())
									{
										obj = 1000;
									}
								}

								paretoSet.push_back(fo);

							}
							paretoSetRef = uND.unionSets(paretoSetRef, paretoSet);
							printVectorPareto(paretoSet, nObj);
							//execl("./hv", ss.str().c_str(), (char *) 0);

							vParetoSet.push_back(paretoSet);
							cout << "Pareto read with size: " << paretoSet.size() << endl << endl;
							//getchar();
						}

						cout << "==================================================" << endl << endl;
					}

		cout << "==================================================" << endl;
		cout << "============" << vParetoSet.size() << " PARETO FRONTS WERE READ============" << endl;
		for (int obj = 0; obj < nObj; obj++)
		{
			maxEval[obj] += 1;
			minEval[obj] -= 1;
		}
		cout << "maxEval[0]=" << maxEval[0] << endl;
		cout << "maxEval[1]=" << maxEval[1] << endl;
		cout << "maxEval[2]=" << maxEval[2] << endl;
		cout << "minEval[0]=" << minEval[0] << endl;
		cout << "minEval[1]=" << minEval[1] << endl;
		cout << "minEval[2]=" << minEval[2] << endl;
		cout << "==================================================" << endl << endl;
		//getchar();
		//Calcula indicadores

		cout << "==================================================" << endl;
		cout << "============ TIME FOR ANALYZING OBTAINED FRONTS ============" << endl;
		cout.precision(10);
		cout<<setprecision(10);
		//INCLUDING PARETO REFERENCE
//		vParetoSet.push_back(paretoSetRef);
		vector<vector<double> > vIndicadoresQualidade;

		for (int nPS = 0; nPS < vParetoSet.size(); nPS++)
		{
			cout << "==================================================" << endl;
			cout << "analyzing file: " << vParetoNames[nPS].c_str() << endl;
			cout << nPS << "/" << (vParetoSet.size() - 1) << " is being analyzed" << endl;

			vector<vector<double> > paretoSet = vParetoSet[nPS];
			vector<double> indicadores;
			indicadores.push_back(paretoSet.size());
			vector<double> utopicPoints = minEval;
			vector<double> hvReferencePoints = maxEval;

			cout << "Cardinalite = " << uND.cardinalite(paretoSet, paretoSetRef) << endl;
//			cout << "Cardinalite = " << uND.cardinalite(paretoSet, paretoSetRef) << endl;
			cout << "uND.setCoverage(PF, ref)  = " << uND.setCoverage(paretoSet, paretoSetRef) << endl;
			cout << "uND.setCoverage(ref, PF)  = " << uND.setCoverage(paretoSetRef, paretoSet) << endl;
//			cout << "uND.setCoverage(PF2, PF1)  = " << uND.setCoverage(paretoSet, paretoSetRef) << endl;
//			cout << "uND.setCoverage(PF1, PF2)  = " << uND.setCoverage(paretoSet, paretoSetRef) << endl;
			cout << "deltaRef  = " << uND.deltaMetric(paretoSetRef, utopicPoints) << endl;
			cout << "delta  = " << uND.deltaMetric(paretoSet, utopicPoints) << endl;
//			cout << "delta  = " << uND.deltaMetric(paretoSet, utopicPoints) << endl;
			cout << "hvRef  = " << hipervolume(paretoSetRef, hvReferencePoints) << endl;
			cout << "hv  = " << hipervolume(paretoSet, hvReferencePoints) << endl;

			//printVectorPareto(paretoSetRef);
			//cout << "conjuntoParetoAtual:" << endl;
			//printVectorPareto(paretoSet);
//			double cardinalidade = uND.cardinalite(paretoSet, paretoSetRef);
//			double setCover = uND.setCoverage(paretoSet, paretoSetRef);
//			indicadores.push_back(cardinalidade);
//			indicadores.push_back(setCover);
//			double sP1 = uND.spacing(paretoSet);
//			indicadores.push_back(sP1);
//
//			double delta = uND.deltaMetric(paretoSet, utopicSol);
//			indicadores.push_back(delta);
//			vIndicadoresQualidade.push_back(indicadores);
			/*			cout << paretoSet.size() << endl;
			 cout << cardinalidade << endl;
			 cout << setCover << endl;
			 cout << sP1 << endl;
			 getchar();*/

		}
		cout << "============ PARETO FRONTS ANALAZED -- PRINTED AND EXPORTED (TODO) ============" << endl;
		cout << "==================================================" << endl << endl;

		cout << "The obtained pareto REF was:" << endl;
		printVectorPareto(paretoSetRef, nObj);
//		cout << "printing Indicadores" << endl;

//		printVectorPareto(vIndicadoresQualidade, nObj);

		cout << "Reading Pareto Sets Ended!" << endl;
		exit(1);
	}

// 2

};

#endif /* READPARETOSETS_HPP_ */
