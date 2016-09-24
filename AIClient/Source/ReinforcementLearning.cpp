
#include "ReinforcementLearning.h"
#include <fstream>
#include <iostream>
#include "RunnerOverride.h"


using namespace std;

/**
** Metodo de solução a ser usado TD (temporal difference)
** Especificamente o algorítimo SARSA ( State-Action-Reward-State-Action)
*/
/**/
// Carregar arquivo
ReinforcementLearning::ReinforcementLearning(UnitProblem <double> * nproblem,
											std::string pathControl,
											double epsilon,		// exploração
											double alpha,		// Aprendizado
											double lambda,		// porcentagem de rasto permanente
											double gamma)
{
	random = new Random < double >;
	problem = nproblem;
	hashing = new UNH<double>(random, 1000);
	projector = new TileCoderHashing<double>(hashing, problem->dimension(), 10, 10, true); // 10, 10, true
	toStateAction = new StateActionTilings<double>(projector, problem->getDiscreteActions());
	e = new ATrace<double>(projector->dimension());



	this->alpha = alpha / projector->dimension();
	cout << "altha: " << this->alpha << endl;
	this->gamma = gamma;
	cout << "gamma: " << this->gamma << endl;
	this->lambda = lambda;
	cout << "lambda: " << this->lambda << endl;
	this->epsilon = epsilon;


	sarsa = new Sarsa<double>(this->alpha, this->gamma,
												this->lambda, e);
	this->epsilon = epsilon;
	acting = new EpsilonGreedyPlus<double>(random,
										problem->getDiscreteActions(),
										sarsa,
										this->epsilon
										);
	path = pathControl;

	// loading
	cout << "Loading sarsa\t\t\t";
	cout << "...\t\t";
	string sarsaPath = "S" + path;
	if (is_file_exist(sarsaPath.c_str())) {
		sarsa->resurrect(sarsaPath.c_str());
		cout << "[OK]\n";
	}
	else{
		cout << "Sarsa file does not exist\n";
	}
	control = new SarsaControl<double>(acting, toStateAction, sarsa);
	cout << "Loading control\t\t\t";
	cout << "...\t\t";
	if (is_file_exist(path.c_str())) {
		control->resurrect(pathControl.c_str());
		cout << "[OK]\n";
	}
	else{
		cout << "Control file does not exist\n";
	}

	agent = new LearnerAgent<double>(control);
	

	sim = new RunnerOverride<double>(agent, problem, 5000);

	sim->setVerbose(true);
}

ReinforcementLearning::~ReinforcementLearning()
{
	delete random;
	delete hashing;
	delete projector;
	delete toStateAction;
	delete e;
	delete sarsa;
	delete acting;
	delete control;
	delete agent;
	delete sim;
}


void ReinforcementLearning::newEpsilon(double epsilon){
	this->epsilon = epsilon;
	acting->setEpsilon(this->epsilon);
}

void ReinforcementLearning::evaluate(){
	sim->computeValueFunction();
	control->persist(path.c_str());
	string sarsaPath = "S" + path;
	sarsa->persist(sarsaPath.c_str());
	Vector<double> *w = sarsa->weights();
	string weightsPath = "W" + path;
	w->persist(weightsPath.c_str());
	

}

void ReinforcementLearning::run(){
	sim->step();
}

void ReinforcementLearning::initialize(){

}

void ReinforcementLearning::stateAction(){
	sim->stateAction();
}

double ReinforcementLearning::reward(){
	return sim->reward();
}

void ReinforcementLearning::end(){
	cout << "RLend" << endl;
	control->persist(path.c_str());
}

bool ReinforcementLearning::is_file_exist(string fileName){
	std::ifstream infile(fileName.c_str());
	return infile.good();
}
