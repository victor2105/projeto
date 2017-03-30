#pragma once
#include <iostream>

#include <BWAPI.h>
#include "rl/Action.h"
#include "rl/ControlAlgorithm.h"
#include "rl/RL.h"

#include "EnemiesHealth.h"
#include "Fight.h"

/*
*		Actions: Discrete
*			0 - Fight
*			1 - Reatreat
*		State: Discrete
*			weapon:
*				0 - in cooldown
*				1 - active
*			distanceCE :
*				0 - x <= 25%
*				1 - 25 < x <= 50%
*				2 - 50 < x <= 75%
*				3 - x > 100%
*			numberEUR: int
*			health: int
*
*/

/**/


using namespace RLLib;
using namespace BWAPI;

/*
#define FIGHT 0
#define REATREAT 1
*/

#define nb_var 5
#define nb_discret_action 2
#define nb_continuous_action 1

template < class T >
class UnitProblem : public RLProblem<T> // this class will observe a unit
{
	typedef RLProblem<T> Base;
protected:
	
	BWAPI::Unit unit_experiment;

	
	int last_enemy_units_health;

	int deltaEHealth;
	int deltaHealth;
	int l_agent_health;

	int kills;
	int l_kills;
	const int MaxRange = 5; // Vulture Weapon Range
	const int TileSize = BWAPI::TILEPOSITION_SCALE; // 32 px


	

public:
	int weapon;	// Weapon Cooldown (is in cooldown or not/ 1 or 0)
	int distanceCE;	// Distance to Closest Enemy
	int numberEUR;	// Number of Enemy Units in Range
	int healthState; // Hit point - Health classified into one of {25,50,75,100}%
	int perigo;		 // Nível de ameaça no range;

	int health;

	bool hasWin;
	bool hasLost;
	UnitProblem(Random<T> * random = 0, BWAPI::Unit u = 0):
		RLProblem<T>(random, nb_var, nb_discret_action, nb_continuous_action), weapon(0), distanceCE(0),
		numberEUR(0), health(0), unit_experiment(u)
	{
		for (int i = 0; i < Base::discreteActions->dimension(); i++)
			Base::discreteActions->push_back(i, i);
		//Base::discreteActions->push_back(0, 0.0);
	}
	
	~UnitProblem(){
	}
	

	// Broodwar
	void initialize() {
		hasWin = false;
		hasLost = false;
		perigo = 0;
		l_agent_health = 80;
		deltaEHealth = 0;
		health = 80;
	}

	void step(const Action<T>* a){
		update_actions(a);
	}

	bool endOfEpisode() const {
		if (unit_experiment) {
			if (unit_experiment->getKillCount() == 6 || hasWin) { // Matou 6 unidades inimigas
				return true;
			}
			if (hasLost){
				return true;
			}
		}
		return false;
	}

	T r() const
	{
		int reward = rewardFunction();
		Broodwar->sendText("rewardR %d", reward);
		cout << "reward: " << reward << endl;

		return reward;
	}

	T z() const
	{
		return 0;
	}

	void setUnit(BWAPI::Unit unit){
		unit_experiment = unit;
	}


	void updateTRStep()
	{
		getState();

		printEstado();

		Base::output->o_tp1->setEntry(0, weapon);
		Base::output->o_tp1->setEntry(1, distanceCE);
		Base::output->o_tp1->setEntry(2, numberEUR);
		Base::output->o_tp1->setEntry(3, healthState);
		Base::output->o_tp1->setEntry(4, perigo);

		Base::output->observation_tp1->setEntry(0, weapon);
		Base::output->observation_tp1->setEntry(1, distanceCE);
		Base::output->observation_tp1->setEntry(2, numberEUR);
		Base::output->observation_tp1->setEntry(3, healthState);
		Base::output->observation_tp1->setEntry(4, perigo);
	}

	void draw()
	{

	}

private:

	void update_actions(const Action<T>* act) {
		if (act->id() == 0){ // Fight action
			Broodwar->sendText("Fight");
			cout << "atacar" << endl;
			ActionImp::getInstance().action = ActionImp::FIGHT;
			ActionImp::getInstance().actionFinished = false;
			Fight::getInstance().start();
		}
		else if(act->id() == 1){ // Reatreat action
			Broodwar->sendText("Reatreat");
			cout << "recuar" << endl;
			ActionImp::getInstance().action = ActionImp::REATREAT;
			ActionImp::getInstance().actionFinished = false;
		}
		else{
			std::cout << "ERROR: not recognized action" << std::endl;
		}
	}

	// State Action Reward State Saction

	
	void getState() {
		Unit u = unit_experiment;


		/*
			ToDo
			Atualizar a vida da unidade
			Atualizar a vida dos inimigos
		*/

		// update health
		l_agent_health = health;
		if (!unit_experiment || !unit_experiment->exists())
		{
			health = 0;
		}
		else
		{
			health = u->getHitPoints();
		}
		deltaHealth = l_agent_health - health;
		
		// Update Enimy Health
		deltaEHealth = EnemiesHealth::getInstance().getDeltaHealth();
		EnemiesHealth::getInstance().clean();

		// Weapon State
		weapon = u->getGroundWeaponCooldown();
		if (weapon != 0){
			weapon = 0;
		} else {
			weapon = 1;
		}

		// Hit point - Health classified into one of {25,50,75,100}%
		// healthState é a discretisação de health

		int fullHealth = u->getInitialHitPoints();
		if (health <= 0.25 * fullHealth) healthState = 0;	
		else if (health <= 0.50 * fullHealth) healthState = 1;
		else if (health <= 0.75 * fullHealth) healthState = 2;
		else if (health <= 1 * fullHealth) healthState = 3;

		// Calculating the number of Enemy Units in Range 
		Unitset enemies = u->getUnitsInRadius(MaxRange*TileSize*3, BWAPI::Filter::IsEnemy);
		Unit closestEnemy = nullptr;
		numberEUR = 0;



		
		perigo = 0;
		for (auto &m : enemies) {
			if (m->getDistance(u->getPosition()) <= MaxRange*TileSize)
				numberEUR++;
			if (!closestEnemy || u->getDistance(m) < u->getDistance(closestEnemy))
				closestEnemy = m;
			
			// O nível de perigo é calculado baseado no tempo de recarga das armas dos inimigos
			// Seja A uma arma inimiga e P(A) a funcao que determina o nível de perigo da arma A. Ou seja,
			// caso a arma A esteja em recarga, o nivel de perigo P(A) = 0. Caso contrário P(A) = 1.

			// Portanto, o nível de perigo total é igual ao S(P(Ai))para(i=1 a n). Onde n é o número de inimigos no range de ataque.
			
			if (m->getDistance(u->getPosition()) <= 4 * TileSize + 8){
				if (m->getGroundWeaponCooldown() == 0) {
					perigo++;
				}
			}
		}

		// Distance of Closest Enemy
		distanceCE = 0; // Quatro possibilidades range 0, 1, 2 e 3

		double distance = 9999.0;
		if (closestEnemy){
			distance = u->getDistance(closestEnemy) + 0.0;
		}

		// distanceCE é a discretisação da distancia do inimigo mais próximo

		if (distance <= 0.25 * MaxRange * TileSize) {
			distanceCE = 0;
		}
		else if (distance <= 0.75 * MaxRange * TileSize) {
			distanceCE = 1;
		}
		else if (distance <= 1.2 * MaxRange * TileSize) {
			distanceCE = 2;
		}
		else if (distance > 1.20 * MaxRange * TileSize) {
			distanceCE = 3;
		}



	}

	void printEstado(){
		cout <<"weapon:";
		cout << weapon<<endl;
		cout << "distanceCE:";
		cout << distanceCE << endl;
		cout << "numberEUR:";
		cout << numberEUR << endl;
		cout << "healthState:";
		cout << healthState << endl;
		cout << "perigo:";
		cout << perigo << endl;

	}

	

	/*
	*	@arguments (l_agent_health, health)
	*/
	
	int rewardFunction() const {
		return deltaEHealth - deltaHealth; // +deltaKills * 100;
	}

};