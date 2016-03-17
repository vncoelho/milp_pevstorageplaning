// ===================================
// Main.cpp file generated by OptFrame
// Project EFP
// ===================================
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "MathModelWLAN.hpp"
#include "readParetoSets.hpp"
#include "./OptFrame/Loader.hpp"
#include "./MyProjects/MILPStoragePlanning.h"

using namespace MILPStoragePlanning;
using namespace std;
using namespace optframe;

int main(int argc, char **argv)
{

	int nOfArguments = 4;
	if (argc != (1 + nOfArguments))
	{
		cout << "Parametros incorretos!" << endl;
		cout << "Os parametros esperados sao: \n"
				"1 - instancia \n"
				"2 - MIP START\n" << endl;
		exit(1);
	}
	const char* instancia = argv[1];
	bool mipStart = atoi(argv[2]);
	int nIntervalsCoef = atoi(argv[3]);
	int tLim = atoi(argv[4]);
	string filename = instancia;
	cout << "filename = " << filename << endl;
	cout << "mipStart = " << mipStart << endl;

	RandGenMersenneTwister rg;
	//long  1412730737
	long seed = time(NULL); //CalibrationMode
	seed = 2;
	cout << "Seed = " << seed << endl;
	srand(seed);
	rg.setSeed(seed);

	/*

	 Scanner scanner(new File("cordenadasClientes"));
	 vector<pair<double, double> > coorClientes;


	 while (scanner.hasNext())
	 {
	 double x = scanner.nextDouble();
	 double y = scanner.nextDouble();
	 coorClientes.push_back(make_pair(x, y));
	 }
	 cout << coorClientes << endl;

	 int incre = 10;

	 FILE* fAMPL = fopen("saidaAMPLWLan", "w");

	 for (int c = 0; c < 256; c++)
	 {
	 cout << "['C" << c + 1 << "',*,*]: \t";
	 fprintf(fAMPL, "['C%d',*,*]: \t ", c + 1);
	 for (int i = 0; i <= 400; i += incre)
	 {
	 cout << i << "\t";
	 fprintf(fAMPL, "%d \t ", i);
	 }
	 cout << ":=" << endl;
	 fprintf(fAMPL, ":= \n ");

	 for (double i = 0; i <= 400; i = i + incre)
	 {
	 cout << i << "\t";
	 fprintf(fAMPL, "%.0f \t ", i);
	 for (double j = 0; j <= 400; j = j + incre)
	 {
	 double d = sqrt(pow(i - coorClientes[c].first, 2) + pow(j - coorClientes[c].second, 2));
	 cout << d << "\t";
	 fprintf(fAMPL, "%.2f \t ", d);
	 }
	 cout << endl;
	 fprintf(fAMPL, "\n ");
	 }

	 }
	 fclose(fAMPL);
	 getchar();
	 */

	readParetoSets rPS;
	rPS.exec(filename, mipStart, nIntervalsCoef, tLim);
	mathModelWLAN mModel(rg);
//	mModel.analyzeParetoFronts("./ResultadosFronteiras/ParetoFrontInputbWCMNExec27TLim10-bestMIPStart", 68, "./ResultadosFronteiras/ParetoFrontInputbWCMNExec27TLim10", 44);
//	getchar();

	mModel.exec(filename, mipStart, nIntervalsCoef, tLim);

	cout << "Ended com sucesso!" << endl;
}
