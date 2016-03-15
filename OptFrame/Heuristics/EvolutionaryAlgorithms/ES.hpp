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

#ifndef OPTFRAME_ES_HPP_
#define OPTFRAME_ES_HPP_

#include <math.h>
#include <vector>

#include "../../LocalSearch.hpp"
#include "../../NSSeq.hpp"
#include "../../SingleObjSearch.hpp"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "../../Timer.hpp"
namespace optframe
{

//ESTRUTURA DA ESTRATEGIA EVOLUTIVA
//CONSTITUIDA POR PROBABILIDADE DE APLICACAO, N APLICACOES, MOVIMENTO]

template<class R, class ADS = OPTFRAME_DEFAULT_ADS, class DS = OPTFRAME_DEFAULT_DS>
struct Tuple
{
	double pr; // probability
	int nap; // number of applications
	NSSeq<R, ADS, DS>* ns;

	Tuple(double _pr, int _nap, NSSeq<R, ADS, DS>* _ns) :
			pr(_pr), nap(_nap), ns(_ns)
	{
	}
};

//CADA INDIVIDUO EH UM PAR DE SOLUCAO E UMA TUPLE COM O PARAMETROS DA ESTRATEGIA
template<class R, class ADS = OPTFRAME_DEFAULT_ADS, class DS = OPTFRAME_DEFAULT_DS>
class ES: public SingleObjSearch<R, ADS, DS>
{
private:

	Solution<R, ADS>* sStar;
	Evaluation<DS>* eStar;
	Evaluator<R, ADS, DS>& eval;
	Constructive<R, ADS>& constructive;
	vector<NSSeq<R, ADS, DS>*> vNS;
	LocalSearch<R, ADS, DS>& ls;
	RandGen& rg;

	int nParam;
	const int mi;
	const int lambda;
	const int gMax;
	int iterSemMelhora;
	int gAtual;

	typedef pair<Solution<R, ADS>*, vector<Tuple<R, ADS, DS> >*> Individuo;

	typedef vector<Individuo> Populacao;

	static bool compara(pair<Individuo, double> p1, pair<Individuo, double> p2)
	{
		return p1.second < p2.second;
	}

	double rand_normal_continuo(double media, double desvpad)
	{

		const gsl_rng_type * RGSL;
		gsl_rng * r;
		gsl_rng_env_setup();
		RGSL = gsl_rng_default;
		r = gsl_rng_alloc(RGSL);

		//gsl_rng * r = gsl_rng_alloc(gsl_rng_ranlux);

		double y;
		y = media + gsl_ran_gaussian(r, desvpad);

		gsl_rng_free(r);

		return y;
	}

	int rand_binomial(double media, double desvpad)
	{
		const gsl_rng_type * RGSL;
		gsl_rng * r;
		gsl_rng_env_setup();
		RGSL = gsl_rng_default;
		r = gsl_rng_alloc(RGSL);

		//gsl_rng * r = gsl_rng_alloc(gsl_rng_ranlux);

		int y;
		y = gsl_ran_binomial(r, 0.5, desvpad);
		gsl_rng_free(r);

		return y;
	}

	double rand_unif() // not zero!
	{
		double r = (rand() % 1000) / 1000.0;
		if (r == 0)
			r += 0.0001;
		return r;
	}

	double rand_nap()
	{
		double r = (rand() % 10) + 1;
		return r;
	}

	double rand_double()
	{

		double normal_media_0 = rand_unif() - 0.5;

		return normal_media_0 / 3.5;
	}

	int rand_int()
	{
		int r = rand() % 8;
		if (r == 1)
			return 1;
		if (r == 2)
			return 1;
		if (r == 3)
			return 2;
		if (r == 4)
			return 3;
		if (r == 5)
			return -1;
		if (r == 6)
			return -1;
		if (r == 7)
			return -2;
		if (r == 8)
			return -3;

		return 0;
	}

	//FUNCAO UTILZIADA NO QuickSort

	void mutaParametros(vector<Tuple<R, ADS, DS> >* p)
	{

		for (int param = 0; param < nParam; param++) // 8 vizinhancas
		{
			//p->at(param).pr += rand_double();
			p->at(param).pr += rg.randG(0,0.5);

			//Verifica a faixa de [0,1] dos parametros
			if (p->at(param).pr < 0)
				p->at(param).pr = 0;
			if (p->at(param).pr > 1)
				p->at(param).pr = 1;

			p->at(param).nap += rand_int();

			if (p->at(param).nap < 0)
				p->at(param).nap = 0;
			if (p->at(param).nap > 15)
				p->at(param).nap = 15;
		}
	}

	void mutaParametrosDistribuicoes(vector<Tuple<R, ADS, DS> >* p, double sigmaC, double sigmaD)
	{

		for (int param = 0; param < nParam; param++) // 8 vizinhancas
		{
			p->at(param).pr += rand_normal_continuo(0, sigmaC);

			//Verifica a faixa de [0,1] dos parametros
			if (p->at(param).pr < 0)
				p->at(param).pr = 0;
			if (p->at(param).pr > 1)
				p->at(param).pr = 1;

			p->at(param).nap += rand_binomial(0.5, sigmaD);

			if (p->at(param).nap < 0)
				p->at(param).nap = 0;
			if (p->at(param).nap > 15)
				p->at(param).nap = (rand() % 10) + 1;
		}
	}

	void aplicaParametros(Solution<R, ADS>* s, vector<Tuple<R, ADS, DS> >* p)
	{

		for (int param = 0; param < nParam; param++) // 8 vizinhancas
		{
			double rx = rand_unif();
			if (rx < p->at(param).pr)
				for (int a = 1; a <= p->at(param).nap; a++)
				{
					Move<R, ADS, DS>* mov_tmp = &p->at(param).ns->move(*s);

					if (mov_tmp->canBeApplied(*s))
					{
						Move<R, ADS, DS>* mov_rev = &mov_tmp->apply(*s);
						delete mov_rev;
					}
					else
					{
						//cout << ".";
					}

					delete mov_tmp;
				}
		}

	}

public:

	ES(Evaluator<R, ADS, DS>& _eval, Constructive<R, ADS>& _constructive, vector<NSSeq<R, ADS, DS>*> _vNS, LocalSearch<R, ADS, DS>& _ls, RandGen& _rg, int _mi, int _lambda, int _gMax) :
			eval(_eval), constructive(_constructive), vNS(_vNS), ls(_ls), rg(_rg), mi(_mi), lambda(_lambda), gMax(_gMax)
	{
		nParam = vNS.size();
		sStar = NULL;
		eStar = NULL;

		iterSemMelhora = 0;
		gAtual = 0;
	}

	virtual ~ES()
	{
	}

	/*
	 void applyLocalSearch(Populacao& p, int nBuscas, int lambda)
	 {

	 bool aux[lambda];
	 for (int i = 0; i++; i < lambda)
	 aux[i] = false;
	 int n = 0;

	 while (n < nBuscas)
	 {
	 int ind;
	 ind = rand() % lambda;

	 if (aux[ind] == false)
	 {

	 Solution<R, ADS>* filhoo = vnd.search(p[ind].first);
	 delete p[ind].first;
	 p[ind].first = filhoo;

	 aux[ind] = true;
	 n++;
	 }
	 }
	 }*/

	void aplicaBuscaLocalBests(Populacao& p, int nBuscas)
	{

		bool aux[nBuscas];
		for (int i = 0; i < nBuscas; i++)
			aux[i] = false;

		//ORDECAO QuickSort
		vector<pair<Individuo, double> > v;

		for (int i = 0; i < p.size(); i++)
		{
			Evaluation<DS>& e = eval.evaluate(*p[i].first);
			v.push_back(make_pair(p[i], e.evaluation()));
			delete &e;
		}

		sort(v.begin(), v.end(), compara); // ordena com QuickSort

		int n = 0;

		while (n < nBuscas)
		{
			int ind;
			ind = rand() % nBuscas;

			if (aux[ind] == false)
			{

				//EvaluationOPM* e = eval.evaluate(p[ind].first);

				//cout << "VALOR DA FO ANTES DA BL= " << e->evaluation() << endl;

				Solution<R, ADS>* filhoo = &ls.search(*p[ind].first);
				delete p[ind].first;
				p[ind].first = filhoo;

				//EvaluationOPM* ee = eval.evaluate(p[ind].first);
				//cout << "VALOR DA FO DPS DA BL= " << ee->evaluation() << endl;

				//delete e;
				//delete ee;

				aux[ind] = true;
				n++;
			}
		}
	}

	Populacao& competicao(Populacao& pais, Populacao& filhos)
	{
		vector<pair<Individuo, double> > v;

		for (int i = 0; i < pais.size(); i++)
		{
			Evaluation<DS>& e = eval.evaluate(*pais[i].first);
			v.push_back(make_pair(pais[i], e.evaluation()));
			delete &e;
		}

		for (int i = 0; i < filhos.size(); i++)
		{
			Evaluation<DS>& e = eval.evaluate(*filhos[i].first);
			v.push_back(make_pair(filhos[i], e.evaluation()));
			delete &e;
		}

		sort(v.begin(), v.end(), compara); // ordena com QuickSort

		Populacao* p = new Populacao;

		double fo_pop = 0;

		for (int i = 0; i < v.size(); i++)
			if (i < mi)
			{
				p->push_back(v[i].first);
				fo_pop += v[i].second;
			}
			else
			{
				delete v[i].first.first; // Solution
				delete v[i].first.second; // vectors de mutacao e prob
			}

		fo_pop = fo_pop / mi;
		//cout << "Media Competicao, media: " << fo_pop << endl;

		//AVALIA MELHOR INDIVIDUO
		double fo;
		fo = v[0].second;

		if (eval.betterThan(fo, eStar->evaluation()))
		{
			//verificar se algo deve ser apagado todo
			sStar = &v[0].first.first->clone();
			eStar = &eval.evaluate(*sStar);

			cout << "Iter:" << gAtual << "\tIterSemMelhora: " << iterSemMelhora;
			cout << "\t Best: " << v[0].second;
			cout << "\t [";
			for (int param = 0; param < nParam; param++)
			{
				cout << "(" << p->at(0).second->at(param).pr;
				cout << ",";
				cout << p->at(0).second->at(param).nap << ") ";
			}
			cout << "]" << endl;

			FILE* arquivo = fopen("logParam.txt", "a");
			if (!arquivo)
			{
				cout << "ERRO: falha ao criar arquivo \"logParam.txt\"" << endl;
			}
			else
			{
				fprintf(arquivo, "%d\t", gAtual);
				for (int param = 0; param < nParam; param++)
				{
					fprintf(arquivo, "%.4f\t%d\t", p->at(0).second->at(param).pr, p->at(0).second->at(param).nap);
				}
				fprintf(arquivo, "\n");
				fclose(arquivo);
			}

			iterSemMelhora = 0;

		}
		else
			iterSemMelhora++;

		return *p;
	}

	Populacao& lowSelectivePression(Populacao& pop, Populacao& pop_filhos)
	{
		//onlyOffsprings
		Populacao pop_nula;
		Populacao& pNova = competicao(pop_nula, pop_filhos);

		for (int i = 0; i < pop.size(); i++)
		{
			delete pop[i].first;
			delete pop[i].second;
		}

		pop_nula.clear();

		return pNova;
	}

	Populacao& highSelectivePression(Populacao& pop, Populacao& pop_filhos)
	{
		Populacao& pNova = competicao(pop, pop_filhos);

		return pNova;
	}

	virtual void localSearch(Solution<R, ADS>& s, Evaluation<DS>& e, double timelimit, double target_f)
	{
		ls.exec(s, e, timelimit, target_f);
	}

	pair<Solution<R, ADS>&, Evaluation<DS>&>* search(double timelimit = 100000000, double target_f = 0, const Solution<R, ADS>* _s = NULL, const Evaluation<DS>* _e = NULL)
	{
		cout << "ES search(" << target_f << "," << timelimit << ")" << endl;

		Timer tnow;

		Populacao pop(mi);
		double fo_inicial = 0;

		//GERANDO VETOR DE POPULACAO INICIAL
		for (int i = 0; i < mi; i++)
		{
			//PartialGreedyInitialSolutionOPM is(opm, 0.4, 0.4, 0.4); // waste, ore, shovel
			Solution<R, ADS>* s = &constructive.generateSolution();
			vector<Tuple<R, ADS, DS> >* m = new vector<Tuple<R, ADS, DS> >;

			for (int nNS = 0; nNS < vNS.size(); nNS++)
			{
				m->push_back(Tuple<R, ADS, DS>(rand_unif(), rand_nap(), vNS[nNS])); //Movement Load
			}

			pop[i] = make_pair(s, m);

			Evaluation<DS>& e = eval.evaluate(*s);
			fo_inicial += e.evaluation();

			if (i == 0)
			{
				eStar = &e;
				sStar = &pop[i].first->clone();
				//cout<<"e.evaluation() = "<<(double)e.evaluation()<<endl;
				//cout<< " eStar = "<<(double)eStar->evaluation()<<endl;
				//getchar();

			}
			else
			{
				if (eval.betterThan(e.evaluation(), eStar->evaluation()))
				{
					eStar = &e;
					//cout<<"e.evaluation() = "<<(double)e.evaluation()<<endl;
					//getchar();
					//verificar se eh necessario deletar sStar anterior todo
					sStar = &pop[i].first->clone();
				}
			}

			//delete &e;

		}

		cout << "Valor Medio das FO's da POP inicial: " << fo_inicial / mi << endl;
		cout << " eStar = " << (double) eStar->evaluation() << endl;
		// ===============================

		//int gAtual = 0;

		//INICIA PARAMETROS DE MUTACAO
		double sigmaC = 0.1;
		double sigmaD = 5;

		iterSemMelhora = 0;

		while ((iterSemMelhora < gMax) && ((tnow.now()) < timelimit) && eval.betterThan(target_f, eStar->evaluation()))
		{
			Populacao pop_filhos;
			double fo_filhos = 0;

			//GERA OS OFFSPRINGS
			for (int l = 1; l <= lambda; l++)
			{
				int x = rand() % mi;

				// Cria Filho e Tuple de Parametros (pi,nap,vizinhança)
				Solution<R, ADS>* filho = &pop[x].first->clone();
				vector<Tuple<R, ADS, DS> >* vt = new vector<Tuple<R, ADS, DS> >(*pop[x].second);

				// Mutacao dos parametros l
				mutaParametros(vt);

				//mutaParametrosDistribuicoes(vt, sigmaC, sigmaD);

				// application dos parametros para gerar filho completo
				aplicaParametros(filho, vt);

				// Busca Local em cada Filho
				//TODO

				// Sem Busca Local
				Solution<R, ADS>* filho_bl = filho;

				pop_filhos.push_back(make_pair(filho_bl, vt));

				Evaluation<DS>& e = eval.evaluate(*filho_bl);
				fo_filhos += e.evaluation();
				delete &e;

			}

			//cout << "Valor Medio das FO's dos filhos: " << fo_filhos / mi << endl;

			//APLICA B.L VND EM 'nb' INDIVIDUOS DA POP_FILHOS
			//aplicaBuscaLocalBests(pop_filhos, 2);
			//cout<<"Applying local Search ..."<<endl;

			//applyLocalSearch(pop_filhos, nb, lambda);

			//cout<<" local search finished!"<<endl;
			//getchar();
			// ETAPA DE SELECAO - MI,LAMBDA ou MI + LAMBDA // ATUALIZA BEST
			//cout<<"Applying selection ..."<<endl;
			Populacao& pNova = lowSelectivePression(pop, pop_filhos); //Estrategia (Mi,Lamda)
			//Populacao& pNova = highSelectivePression(pop, pop_filhos); //Estrategia (Mi+Lamda)
			//cout<<"selection finished !"<<endl;
			pop.clear();
			pop_filhos.clear();

			pop = pNova;

			//INCREMENTA GERACAO
			gAtual++;

			delete &pNova;
		}

		//BUSCA LOCAL NO MELHOR INDIVIDUO COM TODAS AS 8 VIZINHANÇAS

		/*Solution<R, ADS>* sStarBL = &ls.search(*sStar);
		 Evaluation<DS>& eStarBL = eval.evaluate(*sStarBL);
		 cout << "eStarBL = " << (double) eStarBL.evaluation() << endl;
		 cout << "eStar = " << (double) eStar->evaluation() << endl;*/

		cout << "tnow.now() = " << tnow.now() << " timelimit = " << timelimit << endl;
		cout << "Acabou ES = iterSemMelhor = " << iterSemMelhora << " gMax = " << gMax << endl;
		cout << "target_f = " << target_f << " eStar->evaluation() = " << (double) eStar->evaluation() << endl;
		//getchar();

		Solution<R, ADS>& s = *sStar;
		Evaluation<DS>& e = *eStar;

		//cout<<s.getR();
		//getchar();
		//delete eStar;
		//delete sStar;

		return new pair<Solution<R, ADS>&, Evaluation<DS>&>(s, e);
	}

};

template<class R, class ADS = OPTFRAME_DEFAULT_ADS, class DS = OPTFRAME_DEFAULT_DS>
class ESBuilder: public SingleObjSearchBuilder<R, ADS, DS>
{
public:
	virtual ~ESBuilder()
	{
	}

	virtual SingleObjSearch<R, ADS, DS>* build(Scanner& scanner, HeuristicFactory<R, ADS, DS>& hf, string family = "")
	{
		Evaluator<R, ADS, DS>* eval;
		hf.assign(eval, scanner.nextInt(), scanner.next()); // reads backwards!

		Constructive<R, ADS>* constructive;
		hf.assign(constructive, scanner.nextInt(), scanner.next()); // reads backwards!

		vector<NSSeq<R, ADS, DS>*> hlist;
		hf.assignList(hlist, scanner.nextInt(), scanner.next()); // reads backwards!

		string rest = scanner.rest();

		pair<LocalSearch<R, ADS, DS>*, std::string> method;
		method = hf.createLocalSearch(rest);

		LocalSearch<R, ADS, DS>* h = method.first;

		scanner = Scanner(method.second);

		int mi = scanner.nextInt();
		int lambda = scanner.nextInt();
		int gMax = scanner.nextInt();

		return new ES<R, ADS, DS>(*eval, *constructive, hlist, *h, hf.getRandGen(), mi, lambda, gMax);
	}

	virtual vector<pair<string, string> > parameters()
	{
		vector<pair<string, string> > params;
		params.push_back(make_pair(Evaluator<R, ADS, DS>::idComponent(), "evaluation function"));
		params.push_back(make_pair(Constructive<R, ADS>::idComponent(), "constructive heuristic"));
		stringstream ss;
		ss << NS<R, ADS, DS>::idComponent() << "[]";
		params.push_back(make_pair(ss.str(), "list of local searches"));
		params.push_back(make_pair(LocalSearch<R, ADS, DS>::idComponent(), "local search"));
		params.push_back(make_pair("int", "mi"));
		params.push_back(make_pair("int", "lambda"));
		params.push_back(make_pair("int", "gMax"));

		return params;
	}

	virtual bool canBuild(string component)
	{
		return component == ES<R, ADS, DS>::idComponent();
	}

	static string idComponent()
	{
		stringstream ss;
		ss << SingleObjSearchBuilder<R, ADS, DS>::idComponent() << "ES";
		return ss.str();
	}

	virtual string id() const
	{
		return idComponent();
	}
};

}
#endif /* ES_HPP_ */
