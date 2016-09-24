#pragma once
#include "rl/RL.h"
#include <iostream>

using namespace std;
template <typename T>
class EpsilonGreedyPlus :public Greedy<T>
{
protected:
	Random<T>* random;
	T epsilon;
public:
	EpsilonGreedyPlus(Random<T>* random, Actions<T>* actions, Predictor<T>* predictor,
		const T& epsilon) :
		Greedy<T>(actions, predictor), random(random), epsilon(epsilon)
	{
	}

	const Action<T>* sampleAction()
	{
		if (random->nextReal() < epsilon){
			cout << "testando acoes" << endl;
			return Greedy<T>::actions->getEntry(random->nextInt(Greedy<T>::actions->dimension()));
		}
		else{
			cout << "best action" << Greedy<T>::bestAction << endl;
			return Greedy<T>::bestAction;
		}			
	}

	void setEpsilon(T nepsilon){
		epsilon = nepsilon;
	}

	T pi(const Action<T>* a)
	{
		T probability = (a->id() == Greedy<T>::bestAction->id()) ? T(1) - epsilon : T(0);
		return probability + epsilon / Greedy<T>::actions->dimension();
	}

};