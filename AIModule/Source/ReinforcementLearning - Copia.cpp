
#include "ReinforcementLearning.h"
#include <fstream>
#include <iostream>

using namespace std;

/**
** Metodo de solução a ser usado TD (temporal difference)
** Especificamente o algorítimo SARSA ( State-Action-Reward-State-Action)
*/
/**/
ReinforcementLearning::ReinforcementLearning(UnitProblem <double> * nproblem)
{
	random = new Random < double >;
	problem = nproblem;
	hashing = new UNH<double>(random, 10000);
	projector = new TileCoderHashing<double>(hashing, problem->dimension(), 9, 10,
		false); // 10, 10, true //--> Tentando retirar
	toStateAction = new StateActionTilings<double>(projector, problem->getDiscreteActions());
	e = new ATrace<double>(projector->dimension());
	alpha = 0.05;
	gamma = 0.9;
	lambda = 0.9;
	sarsaAdaptive = new SarsaAlphaBound<double>(alpha, gamma, lambda, e);
	epsilon = 0.9;
	acting = new EpsilonGreedy<double>(random, problem->getDiscreteActions(), sarsaAdaptive, epsilon);
	control = new SarsaControl<double>(acting, toStateAction, sarsaAdaptive);
	
	agent = new LearnerAgent<double>(control);
	sim = new RLRunner<double>(agent, problem, 5000, 300, 1);

	// Tentando carregar o arquivo de controle da ultima seção
	path = "reinforcementLearning.bin";
	if (is_file_exist(path)) {
		cout << "Loading RL data" << endl;
		control->reset();
		control->resurrect(path.c_str());
	}
	else{
		cout << "RL data not found" << endl;
		control = new SarsaControl<double>(acting, toStateAction, sarsaAdaptive);
	}
	sim->setVerbose(true);
}/**/

// Carregar arquivo
ReinforcementLearning::ReinforcementLearning(UnitProblem <double> * nproblem, std::string pathControl)
{
	random = new Random < double >;
	problem = nproblem;
	hashing = new UNH<double>(random, 10000);
	projector = new TileCoderHashing<double>(hashing, problem->dimension(), 9, 10,
		false); // 10, 10, true
	toStateAction = new StateActionTilings<double>(projector, problem->getDiscreteActions());
	e = new ATrace<double>(projector->dimension());
	gamma = 0.99;
	lambda = 0.3;
	sarsaAdaptive = new SarsaAlphaBound<double>(1.0f, gamma, lambda, e);//new SarsaAlphaBound<double>(1.0f, gamma, lambda, e);
	epsilon = 0.01;
	acting = new EpsilonGreedy<double>(random, problem->getDiscreteActions(), sarsaAdaptive, epsilon);
	control = new SarsaControl<double>(acting, toStateAction, sarsaAdaptive);

	agent = new LearnerAgent<double>(control);
	sim = new RLRunner<double>(agent, problem, 5000, 300, 1);

	// Tentando carregar o arquivo da tabela de controle da ultima seção
	path = pathControl;
	if (is_file_exist(path)) {
		cout << "Loading RL data" << endl;
		control->reset();
		control->resurrect(pathControl.c_str());
	}
	else{
		cout << "RL data not found" << endl;
		control = new SarsaControl<double>(acting, toStateAction, sarsaAdaptive);
	}
	sim->setVerbose(true);
}

ReinforcementLearning::~ReinforcementLearning()
{

	delete random;
	delete problem;
	delete hashing;
	delete projector;
	delete toStateAction;
	delete e;
	delete sarsaAdaptive;
	delete acting;
	delete control;
	delete agent;
	delete sim;
}

void ReinforcementLearning::evaluate(){
	
}

void ReinforcementLearning::run(){
	sim->step();
}

void ReinforcementLearning::end(){
	control->persist(path.c_str());
	sim->computeValueFunction();
}

/**/

bool ReinforcementLearning::is_file_exist(string fileName){
	std::ifstream infile(fileName.c_str());
	return infile.good();
}