#pragma once
#include <BWAPI.h>
#include "UnitProblem.h"
#include "ReinforcementLearning.h"
#include "Reatreat.h"
#include <queue>

// Remember not to use "Broodwar" in any global class constructor!


class RLAIModule : public BWAPI::AIModule
{
public:
  // Construtor
  RLAIModule();

  // Virtual functions for callbacks, leave these as they are.
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onSendText(std::string text);
  virtual void onReceiveText(BWAPI::Player player, std::string text);
  virtual void onPlayerLeft(BWAPI::Player player);
  virtual void onNukeDetect(BWAPI::Position target);
  virtual void onUnitDiscover(BWAPI::Unit unit);
  virtual void onUnitEvade(BWAPI::Unit unit);
  virtual void onUnitShow(BWAPI::Unit unit);
  virtual void onUnitHide(BWAPI::Unit unit);
  virtual void onUnitCreate(BWAPI::Unit unit);
  virtual void onUnitDestroy(BWAPI::Unit unit);
  virtual void onUnitMorph(BWAPI::Unit unit);
  virtual void onUnitRenegade(BWAPI::Unit unit);
  virtual void onSaveGame(std::string gameName);
  virtual void onUnitComplete(BWAPI::Unit unit);
  // Everything below this line is safe to modify.
private:
	ReinforcementLearning * reinforcement;
	UnitProblem<double> * unitProblem;
	void unitControl();
	void dragoons(BWAPI::Unit);
	void zealots(BWAPI::Unit);
	void muta(BWAPI::Unit);
	void vulture(BWAPI::Unit);

	string saveAt;
	string pathRLFile;
	double epsiloni;
	double epsilonf;
	double count;

	double decreaseParam ;
	double deltaE;
	double porcentagem;

	// Aprendisado
	double alpha;
	double lambda;
	double gamma;
	double epsilon;

	// Estatistica
	int rewardCount;
	

	// Executivo
	Reatreat reatreatAction;
	
	
};
