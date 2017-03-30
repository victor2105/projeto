#pragma once

#include "rl/ControlAlgorithm.h"
#include "rl/RL.h"
#include "rl/Vector.h"
#include "UnitProblem.h"
#include "ActionImp.h"
#include "RunnerOverride.h"
#include "EpsilonGreedyPlus.h"

using namespace RLLib;

class ReinforcementLearning
{
public:
	ReinforcementLearning(UnitProblem <double> * nproblem,
		std::string pathControl = "reinforcementLearning.bin",
		double epsilon=0.01,	// exploração
		double alpha=0.05,		// Aprendizado
		double lambda=0.9,		// porcentagem de rasto permanente
		double gamma=0.9);
	~ReinforcementLearning();

	void run();

	void initialize();
	void stateAction();
	double reward();

	void end();
	void evaluate();

	void newEpsilon(double epsilon);

private:

	Random <double> * random;
	//RP Problem
	RLProblem <double> * problem;
	// Projector
	Hashing <double> * hashing;
	Projector <double> * projector;
	StateToStateAction <double> * toStateAction;
	// Predictor
	Sarsa<double> * sarsa;			// SARSA
	Trace<double> * e;				// TRACE
	double alpha;
	double gamma;
	double lambda;					// LAMBDA
	double epsilon;					// EPSILON
	EpsilonGreedyPlus<double>* acting;

	// Controller
	OnPolicyControlLearner<double>* control;

	// Runner
	RLAgent<double> * agent;
	RunnerOverride <double> * sim;

	string path;
	bool is_file_exist(string fileName);
};