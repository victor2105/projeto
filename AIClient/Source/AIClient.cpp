#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <BWAPI.h>
#include <BWAPI/Client.h>

#include "ReinforcementLearning.h"
#include "UnitProblem.h"
#include "FileReader.h"
#include "FileWriter.h"

// Implementa��o das a��es
#include "ActionImp.h"
#include "Fight.h"
#include "Reatreat.h"
#include "ExplorerAgent.h"
#include "EnemiesHealth.h"

using namespace BWAPI;
using namespace std;


void drawStats();
void drawBullets();
void drawVisibilityData();
void showPlayers();
void showForces();
bool show_bullets;
bool show_visibility_data;
void initRL();

void reconnect()
{
  while(!BWAPIClient.connect())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
  }
}

int main(int argc, const char* argv[])
{
  std::cout << "Connecting..." << std::endl;
  reconnect();

  ExplorerAgent explorerAgent;
  UnitProblem<double> * unit_problem = 0;
  ReinforcementLearning * reinforcement = 0;
  Reatreat reatreatAction;


  bool enemy_unit_discovered = false;
  Unit unit_experiment = nullptr;

  /*
	1 Leitura de configura��o
  */

  string fileName = "";
  if (argc > 1){
	  fileName = argv[1];
  }
  else{
	  return 0;
  }


  FileReader file(fileName.c_str());
  string saveAt = "results"+fileName;
  double epsiloni = file.getDouble("epsiloni");
  double epsilonf = file.getDouble("epsilonf");
  double count = file.getDouble("count");

  double alpha = file.getDouble("alpha");
  double lambda = file.getDouble("lambda");
  double gamma = file.getDouble("gamma");

  double decreaseParam = file.getDouble("decr");
  int boLinear = file.getInt("boLinear");
  int boPerigo = file.getInt("boPerigo");

  double deltaE =  (epsilonf - epsiloni) / count;
  //double porcentagem = 0.01;
  double porcentagem = decreaseParam;
  /**
	A ultima vers�o utilisando o deltaE foi um desastre. Sendo assim, acredito que essa foi a abordagem utilizada
	no artigo. Entretanto, a forma correta de reduzir o epsilon ainda � um grande mist�rio.
  */
  double epsilon = epsiloni;
  double rewardCount = 0;


  while(true)
  {
    std::cout << "waiting to enter match" << std::endl;
    while ( !Broodwar->isInGame() )
    {
      BWAPI::BWAPIClient.update();
      if (!BWAPI::BWAPIClient.isConnected())
      {
        std::cout << "Reconnecting..." << std::endl;
        reconnect();
      }
    }
    std::cout << "starting match!" << std::endl;
	// O epsilon vai variar de 0.9 at� 0 em um periodo de 1000 epis�dios
	string pathRLFile = "rl.txt";
	
	pathRLFile = "ctrl"+fileName;
	cout << pathRLFile << endl;	

	cout << "inicio" << endl;

	Fight::getInstance().clear();
	EnemiesHealth::getInstance().reset();
	reatreatAction.clear();
	ActionImp::getInstance().clear();

	/*
		2 Inicializa��o e atualiza��o das configura��es do aprendizado
		onde instanciamos, no primeiro epis�dio, as classes UnitProblem e ReinforcementLearning, passando
		os devidos par�metros ($\alhpa$, $\gamma$, $\lambda$, $\epsilon_i$, a inst�ncia do unitProblem
		e o caminho para os arquivos de persist�ncia de informa��o). Nos epis�dios seguintes, apenas
		atualizamos o $\epsilon$ com seu novo valor.
	*/
	if (!reinforcement){
		if (boPerigo == 1)
			unit_problem = new UnitProblem<double>(5);
		else
			unit_problem = new UnitProblem<double>(4);
		reinforcement = new ReinforcementLearning(unit_problem, pathRLFile, epsilon, alpha,lambda,gamma);
	}
	else{
		if (boLinear == 1){
			epsilon += deltaE;
		}
		else{
			epsilon = epsilon * (1.0 - porcentagem);
		}
		if (epsilon < 0) epsilon = 0;
		reinforcement->newEpsilon(epsilon);
	}

	cout << "Epsilon: " << epsilon << endl;
	rewardCount = 0;


	Broodwar << "The map is " << Broodwar->mapName() << ", a " << Broodwar->getStartLocations().size() << " player map" << std::endl;
    // Enable some cheat flags
    Broodwar->enableFlag(Flag::UserInput);
    // Uncomment to enable complete map information
    //Broodwar->enableFlag(Flag::CompleteMapInformation);

    show_bullets=false;
    show_visibility_data=false;

    if (Broodwar->isReplay())
    {
      Broodwar << "The following players are in this replay:" << std::endl;;
      Playerset players = Broodwar->getPlayers();
      for(auto p : players)
      {
        if ( !p->getUnits().empty() && !p->isNeutral() )
          Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
      }
    }
    else
    {
      if (Broodwar->enemy())
        Broodwar << "The match up is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;

      //send each worker to the mineral field that is closest to it
      Unitset units    = Broodwar->self()->getUnits();
      Unitset minerals  = Broodwar->getMinerals();
      for ( auto &u : units )
      {
        if ( u->getType().isWorker() )
        {
          Unit closestMineral = nullptr;

          for (auto &m : minerals)
          {
            if ( !closestMineral || u->getDistance(m) < u->getDistance(closestMineral))
              closestMineral = m;
          }
          if ( closestMineral )
            u->rightClick(closestMineral);
        }
        else if ( u->getType().isResourceDepot() )
        {
          //if this is a center, tell it to build the appropiate type of worker
          //u->train(Broodwar->self()->getRace().getWorker());
		}
		else if (u->getType() == UnitTypes::Terran_Vulture){
			unit_experiment = u;
			if (unit_problem){
				unit_problem->setUnit(u);
				unit_problem->initialize();
				Fight::getInstance().setUnit(u);
				EnemiesHealth::getInstance().setUnit(u);
				reatreatAction.setUnit(u);
				explorerAgent.setUnit(u);
			}
		}

      }
    }
    while(Broodwar->isInGame())
    {
	

	  for(auto &e : Broodwar->getEvents())
      {
        switch(e.getType())
        {
          case EventType::MatchEnd:
			enemy_unit_discovered = false;

			cout << "fim" << endl;

			// TODO: escrever arquivo com os dados de vitoria e derrota
			if (e.isWinner()){
				unit_problem->hasWin = true;
				rewardCount += reinforcement->reward();
				//reinforcement->evaluate();
				//reinforcement->end();
				Broodwar << "I won the game" << std::endl;
				FileWriter fw;
				fw.begin(saveAt);
				fw.writeString("win");
				fw.space();
				fw.writeDouble(count);
				fw.space();
				fw.writeDouble(epsilon);
				fw.space();
				cout << "final reward: " << rewardCount << endl;
				fw.writeDouble(rewardCount);
				rewardCount = 0;
				fw.end();
				count--;
				cout << "\nwin" << endl;
				if (count <= 0){
					reinforcement->evaluate();
					reinforcement->end();
					delete reinforcement;
					delete unit_problem;
					system("fimDosTestes.vbs");
					//system("pause");
					exit(0);
				}

			}
			else{
				unit_problem->hasLost = true;
				rewardCount += reinforcement->reward();
				//reinforcement->evaluate();

				//reinforcement->end();
				Broodwar << "I lost the game" << std::endl;
				FileWriter fw;
				cout << "final reward: " << rewardCount << endl;
				fw.begin(saveAt);
				fw.writeString("lost");
				fw.space();
				fw.writeDouble(count);
				fw.space();
				fw.writeDouble(epsilon);
				fw.space();
				fw.writeDouble(rewardCount);
				rewardCount = 0;
				fw.end();
				count--;
				cout << "\nlost" << endl;
				if (count <= 0){
					reinforcement->evaluate();
					reinforcement->end();
					delete reinforcement;
					delete unit_problem;
					system("fimDosTestes.vbs");
					//system("pause");
					exit(0);
				}
			}
            break;
          case EventType::SendText:
            if (e.getText()=="/show bullets")
            {
              show_bullets=!show_bullets;
            } else if (e.getText()=="/show players")
            {
              showPlayers();
            } else if (e.getText()=="/show forces")
            {
              showForces();
            } else if (e.getText()=="/show visibility")
            {
              show_visibility_data=!show_visibility_data;
            }
            else
            {
              Broodwar << "You typed \"" << e.getText() << "\"!" << std::endl;
            }
            break;
          case EventType::ReceiveText:
            Broodwar << e.getPlayer()->getName() << " said \"" << e.getText() << "\"" << std::endl;
            break;
          case EventType::PlayerLeft:
            Broodwar << e.getPlayer()->getName() << " left the game." << std::endl;
            break;
          case EventType::NukeDetect:
            if (e.getPosition()!=Positions::Unknown)
            {
              Broodwar->drawCircleMap(e.getPosition(), 40, Colors::Red, true);
              Broodwar << "Nuclear Launch Detected at " << e.getPosition() << std::endl;
            }
            else
              Broodwar << "Nuclear Launch Detected" << std::endl;
            break;
          case EventType::UnitCreate:
            if (!Broodwar->isReplay())
              Broodwar << "A " << e.getUnit()->getType() << " [" << e.getUnit() << "] has been created at " << e.getUnit()->getPosition() << std::endl;
            else
            {
              // if we are in a replay, then we will print out the build order
              // (just of the buildings, not the units).
              if (e.getUnit()->getType().isBuilding() && e.getUnit()->getPlayer()->isNeutral()==false)
              {
                int seconds=Broodwar->getFrameCount()/24;
                int minutes=seconds/60;
                seconds%=60;
                Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, e.getUnit()->getPlayer()->getName().c_str(), e.getUnit()->getType().c_str());
              }
            }
            break;
		
          case EventType::UnitDestroy:
			  if (!Broodwar->isReplay()){
				  Broodwar->sendText("A %s [%p] has been destroyed at (%d,%d)", e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPosition().x, e.getUnit()->getPosition().y);
				  
				  EnemiesHealth::getInstance().unitDestroy(e.getUnit()->getID());
			  }
               break;
          case EventType::UnitMorph:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%p] has been morphed at (%d,%d)",e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPosition().x, e.getUnit()->getPosition().y);
            else
            {
              // if we are in a replay, then we will print out the build order
              // (just of the buildings, not the units).
              if (e.getUnit()->getType().isBuilding() && e.getUnit()->getPlayer()->isNeutral()==false)
              {
                int seconds=Broodwar->getFrameCount()/24;
                int minutes=seconds/60;
                seconds%=60;
                Broodwar->sendText("%.2d:%.2d: %s morphs a %s" ,minutes, seconds, e.getUnit()->getPlayer()->getName().c_str(), e.getUnit()->getType().c_str());
              }
            }
            break;
          case EventType::UnitShow:
			  if (!Broodwar->isReplay()){
				  Broodwar->sendText("A %s [%p] has been spotted at (%d,%d)", e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPosition().x, e.getUnit()->getPosition().y);
				 // EnemiesHealth::getInstance().unitShow(e.getUnit()->getID(), e.getUnit()->getHitPoints());
				 // Descobrir como identificar o dono da unidade
			  }
            
			break;
          case EventType::UnitHide:
			  if (!Broodwar->isReplay()){
				  Broodwar->sendText("A %s [%p] was last seen at (%d,%d)", e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPosition().x, e.getUnit()->getPosition().y);
				  Unit u = e.getUnit();

				  EnemiesHealth::getInstance().unitHide(u->getID(), u->getHitPoints());

				  
				  // Unidade fora do campo de vis�o
			  }
             
            break;
          case EventType::UnitRenegade:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%p] is now owned by %s", e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPlayer()->getName().c_str());
            break;
          case EventType::SaveGame:
            Broodwar->sendText("The game was saved to \"%s\".", e.getText().c_str());
            break;
        }
      }

      if (show_bullets)
        drawBullets();

      if (show_visibility_data)
        drawVisibilityData();

      drawStats();
      Broodwar->drawTextScreen(300,0,"FPS: %f",Broodwar->getAverageFPS());
	  
	  if (unit_experiment){
		//  cout << "IMPORTANTE" << unit_experiment->getGroundWeaponCooldown() << endl;
		  if (unit_experiment->getGroundWeaponCooldown() <= 10){
			  Fight::getInstance()._canShot = true;
		  }
		  
		  if (Fight::getInstance()._canShot && !Fight::getInstance().hasFinished() && unit_experiment->getGroundWeaponCooldown() >= 20){
			  Fight::getInstance().end();

		  }
	  }
	 
	  // Calculando varia��o na vida dos inimigos
	  // Essa fun��o � de suma importancia para o funcionamento do aprendizado
	  EnemiesHealth::getInstance().calculateDeltahHealth();


	  // Mostar informa��o da unidade na tela do jogo.
	  if (unit_experiment){
		  Broodwar->drawTextScreen(300, 30, "Arma: %d", unit_problem->weapon);
		  Broodwar->drawTextScreen(300, 60, "Distancia do inimigo mais proximo: %d", unit_problem->distanceCE);
		  Broodwar->drawTextScreen(300, 90, "Numero de inimigos no range de ataque: %d", unit_problem->numberEUR);
		  Broodwar->drawTextScreen(300, 120, "Status da vida: %d [%d]", unit_problem->healthState, unit_experiment->getHitPoints());
		  Broodwar->drawTextScreen(300, 150, "Perigo: %d", unit_problem->perigo);

		  if (unit_experiment->getGroundWeaponCooldown() != 0){
			  Broodwar->drawCircleMap(unit_experiment->getPosition(), 5 * 32, BWAPI::Colors::Red, false);
		  }
		  else{
			  Broodwar->drawCircleMap(unit_experiment->getPosition(), 5 * 32, BWAPI::Colors::Green, false);
		  }

		  Unitset enemies = unit_experiment->getUnitsInRadius(5*32 * 3, BWAPI::Filter::IsEnemy);

		  for (auto &m : enemies) {
			  if (m->getGroundWeaponCooldown() != 0){
				  Broodwar->drawCircleMap(m->getPosition(), 5 * 32, BWAPI::Colors::Red, false);
			  }
			  else{

				  Broodwar->drawCircleMap(m->getPosition(), 5 * 32, BWAPI::Colors::Green, false);
			  }
		  }

	  }

	  if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() == 0){
		  
		  
		 
	  }
	  Unitset enemies = unit_experiment->getUnitsInRadius(5 * 64, BWAPI::Filter::IsEnemy);

	  if (enemies.size() != 0){
		  enemy_unit_discovered = true;
	  }

	  /*
		Controle da Unidade
		 Esta etapa � o cora��o do modulo AIClient. Ela � a respons�vel por alternar entre
		 a execu��o das a��es (Atacar e Recuar), e a execu��o do Agente de Aprendizado por
		 refor�o. Esta etapa � executada a cada quadro do jogo.
	  */
	  if (enemy_unit_discovered){

		  // Verificar se a ultima a��o foi finalizada
		  // Atacar
		  // Recuar
		  // se a proxima a��o � atacar e hsasAttacked is false
		  // se unit_experiment->getGroundWeaponCooldown() != 0 hasAttacked = trues
		  if (!ActionImp::getInstance().actionFinished){
			  /**/
			  if (ActionImp::getInstance().action == ActionImp::FIGHT){
				  Fight::getInstance().update();

			  }/**/
			  else  if (ActionImp::getInstance().action == ActionImp::REATREAT){
				  reatreatAction.execute(); // Deve vir sempre ante do teste
				  if (reatreatAction.hasFinished()){
					  //system("cls");
					  //unit_problem->setUnitHealth(unit_experiment->getHitPoints());

					  ActionImp::getInstance().actionFinished = true;
					  ActionImp::getInstance().action = ActionImp::NO_ACTION;
				  }
			  }/**/
		  }
		  if (ActionImp::getInstance().actionFinished){
			  if (unit_problem){
				  //reinforcement->run();
				  reinforcement->reward();

				  reinforcement->stateAction();
			  }
			  else{
				  Broodwar << "unit problem = null" << std::endl;
			  }
		  }
	  }
	  else{
		  explorerAgent.execute();
	  }
		  
	  
      BWAPI::BWAPIClient.update();
      if (!BWAPI::BWAPIClient.isConnected())
      {
        std::cout << "Reconnecting..." << std::endl;
        reconnect();
      }
    }
    std::cout << "Game ended" << std::endl;
  }
  std::cout << "Press ENTER to continue..." << std::endl;
  std::cin.ignore();
  return 0;
}

void drawStats()
{
  int line = 0;
  Broodwar->drawTextScreen(5, 0, "I have %d units:", Broodwar->self()->allUnitCount() );
  for (auto& unitType : UnitTypes::allUnitTypes())
  {
    int count = Broodwar->self()->allUnitCount(unitType);
    if ( count )
    {
      Broodwar->drawTextScreen(5, 16*line, "- %d %s%c", count, unitType.c_str(), count == 1 ? ' ' : 's');
      ++line;
    }
  }

}

void drawBullets()
{
  for (auto &b : Broodwar->getBullets())
  {
    Position p = b->getPosition();
    double velocityX = b->getVelocityX();
    double velocityY = b->getVelocityY();
    Broodwar->drawLineMap(p, p + Position((int)velocityX, (int)velocityY), b->getPlayer() == Broodwar->self() ? Colors::Green : Colors::Red);
    Broodwar->drawTextMap(p, "%c%s", b->getPlayer() == Broodwar->self() ? Text::Green : Text::Red, b->getType().c_str());
  }
}

void drawVisibilityData()
{
  int wid = Broodwar->mapHeight(), hgt = Broodwar->mapWidth();
  for ( int x = 0; x < wid; ++x )
    for ( int y = 0; y < hgt; ++y )
    {
      if ( Broodwar->isExplored(x, y) )
        Broodwar->drawDotMap(x*32+16, y*32+16, Broodwar->isVisible(x, y) ? Colors::Green : Colors::Blue);
      else
        Broodwar->drawDotMap(x*32+16, y*32+16, Colors::Red);
    }
}

void showPlayers()
{
  Playerset players = Broodwar->getPlayers();
  for(auto p : players)
    Broodwar << "Player [" << p->getID() << "]: " << p->getName() << " is in force: " << p->getForce()->getName() << std::endl;
}

void showForces()
{
  Forceset forces=Broodwar->getForces();
  for(auto f : forces)
  {
    Playerset players = f->getPlayers();
    Broodwar << "Force " << f->getName() << " has the following players:" << std::endl;
    for(auto p : players)
      Broodwar << "  - Player [" << p->getID() << "]: " << p->getName() << std::endl;
  }
}