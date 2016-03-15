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


	void exec()
	{
		cout << "Exec Math Model WLAN TC Disciplina Multiobjectivo" << endl;
		UnionNDSets2 uND(2);

		vector<string> vInputModel;
		vInputModel.push_back("arq100m");
		vInputModel.push_back("arq50m");
		vInputModel.push_back("arq10m");
		vector<int> vNMaxOpt;
		vNMaxOpt.push_back(50);
		vNMaxOpt.push_back(100);
		vNMaxOpt.push_back(200);
		vector<int> vTLim;
		vTLim.push_back(20);
		vTLim.push_back(60);

		vector<vector<vector<double> > > vParetoSet;
		vector<vector<double> > paretoSetRef;
		double maxEval1 = 0;
		double maxEval2 = 0;
		double minEval1 = 1000000;
		double minEval2 = 1000000;

		for (int iM = 0; iM < vInputModel.size(); iM++)
			for (int nM = 0; nM < vNMaxOpt.size(); nM++)
				for (int tL = 0; tL < vTLim.size(); tL++)
				{
					stringstream ss;
					ss << "./ResultadosFronteiras/FronteiraPareto" << vInputModel[iM] << "N" << vNMaxOpt[nM] << "T" << vTLim[tL];
					cout << "reading pareto set of model = " << vInputModel[iM] << endl;
					cout << "max = " << vNMaxOpt[nM] << endl;
					cout << "time limit = " << vTLim[tL] << endl;
					Scanner scanner(new File(ss.str()));
					vector<vector<double> > paretoSet;
					while (scanner.hasNext())
					{
						vector<double> fo;
						double eval1 = scanner.nextDouble();
						double eval2 = scanner.nextDouble();
						fo.push_back(eval1);
						fo.push_back(eval2);
						if (eval1 > maxEval1)
							maxEval1 = eval1;
						if (eval2 > maxEval2)
							maxEval2 = eval2;

						if (eval1 < minEval1)
							minEval1 = eval1;
						if (eval2 < minEval2)
							minEval2 = eval2;

						paretoSet.push_back(fo);

					}
					paretoSetRef = uND.unionSets(paretoSetRef, paretoSet);
					printVectorPareto(paretoSet, 2);
					//execl("./hv", ss.str().c_str(), (char *) 0);

					vParetoSet.push_back(paretoSet);
					//getchar();
				}

		cout << "maxEval1=" << maxEval1 << endl;
		cout << "maxEval2=" << maxEval2 << endl;
		cout << "minEval1=" << minEval1 << endl;
		cout << "minEval2=" << minEval2 << endl;
		//getchar();
		//Calcula indicadores
		vParetoSet.push_back(paretoSetRef);

		vector<vector<double> > vIndicadoresQualidade;
		cout << "===================================" << endl;

		for (int nPS = 0; nPS < vParetoSet.size(); nPS++)
		{
			vector<vector<double> > paretoSet = vParetoSet[nPS];
			vector<double> indicadores;
			indicadores.push_back(paretoSet.size());

			//printVectorPareto(paretoSetRef);
			//cout << "conjuntoParetoAtual:" << endl;
			//printVectorPareto(paretoSet);
			double cardinalidade = uND.cardinalite(paretoSet, paretoSetRef);
			double setCover = uND.setCoverage(paretoSet, paretoSetRef);
			indicadores.push_back(cardinalidade);
			indicadores.push_back(setCover);
			double sP1 = uND.spacing(paretoSet);
			indicadores.push_back(sP1);
			vector<double> utopicSol;
			utopicSol.push_back(minEval1);
			utopicSol.push_back(minEval2);
			double delta = uND.deltaMetric(paretoSet, utopicSol);
			indicadores.push_back(delta);
			vIndicadoresQualidade.push_back(indicadores);
			/*			cout << paretoSet.size() << endl;
			 cout << cardinalidade << endl;
			 cout << setCover << endl;
			 cout << sP1 << endl;
			 getchar();*/
		}
		int indexSol = 0;
		for (int iM = 0; iM < vInputModel.size(); iM++)
			for (int nM = 0; nM < vNMaxOpt.size(); nM++)
				for (int tL = 0; tL < vTLim.size(); tL++)
				{
					stringstream ss;
					ss << "./ResultadosFronteiras/FronteiraPareto" << vInputModel[iM] << "N" << vNMaxOpt[nM] << "T" << vTLim[tL];
					stringstream ssHV;
					ssHV << "./hv\t -r \"" << maxEval1 * 1.1 << " " << maxEval2 * 1.1 << "\" \t" << ss.str().c_str();
					cout << ssHV.str() << endl;
					string hvValueString = execCommand(ssHV.str().c_str());
					double hvValue = atof(hvValueString.c_str());
					cout << hvValue;
					vIndicadoresQualidade[indexSol].push_back(hvValue);
					indexSol++;
				}
		cout << "printing pareto REF" << endl;
		printVectorPareto(paretoSetRef,2);
		cout << "printing Indicadores" << endl;
		cout.precision(10);
		printVectorPareto(vIndicadoresQualidade,2);

		getchar();

	}

// 2

};

#endif /* READPARETOSETS_HPP_ */
