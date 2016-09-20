#pragma once
#include "rl/ControlAlgorithm.h"
#include "rl/RL.h"

using namespace RLLib;


template<class T>
class RunnerOverride
{
public:
	class Event
	{
		friend class RunnerOverride;
	protected:
		int nbTotalTimeSteps;
		int nbEpisodeDone;
		T averageTimePerStep;
		T episodeR;
		T episodeZ;

	public:
		Event() :
			nbTotalTimeSteps(0), nbEpisodeDone(0), averageTimePerStep(0), episodeR(0), episodeZ(0)
		{
		}

		virtual ~Event()
		{
		}

		virtual void update() const = 0;

	};
protected:
	RLAgent<T>* agent;
	RLProblem<T>* problem;
	const Action<T>* agentAction;

	int maxEpisodeTimeSteps;
	int nbEpisodes;
	int nbRuns;
	int nbEpisodeDone;
	bool endingOfEpisode;
	bool verbose;

#if !defined(EMBEDDED_MODE)
	Timer timer;
#endif
	T totalTimeInMilliseconds;

#if !defined(EMBEDDED_MODE)
	std::vector<T> statistics;
#endif
	bool enableStatistics;

	bool enableTestEpisodesAfterEachRun;
	int maxTestEpisodesAfterEachRun;
public:
	int timeStep;
	T episodeR;
	T episodeZ;
	std::vector<Event*> onEpisodeEnd;

	RunnerOverride(RLAgent<T>* agent, RLProblem<T>* problem, const int& maxEpisodeTimeSteps,
		const int nbEpisodes = -1, const int nbRuns = -1) :
		agent(agent), problem(problem), agentAction(0), maxEpisodeTimeSteps(maxEpisodeTimeSteps), //
		nbEpisodes(nbEpisodes), nbRuns(nbRuns), nbEpisodeDone(0), endingOfEpisode(false), //
		verbose(true), totalTimeInMilliseconds(0), enableStatistics(false), //
		enableTestEpisodesAfterEachRun(false), maxTestEpisodesAfterEachRun(20), timeStep(0), //
		episodeR(0), episodeZ(0)
	{
	}

	~RunnerOverride()
	{
		onEpisodeEnd.clear();
	}

	void setVerbose(const bool& verbose)
	{
		this->verbose = verbose;
	}

	void setRuns(const int& nbRuns)
	{
		this->nbRuns = nbRuns;
	}

	void setEpisodes(const int& nbEpisodes)
	{
		this->nbEpisodes = nbEpisodes;
	}

	void setEnableStatistics(const bool& enableStatistics)
	{
		this->enableStatistics = enableStatistics;
	}

	void setTestEpisodesAfterEachRun(const bool& enableTestEpisodesAfterEachRun)
	{
		this->enableTestEpisodesAfterEachRun = enableTestEpisodesAfterEachRun;
	}

	void benchmark()
	{
#if !defined(EMBEDDED_MODE)
		T xbar = std::accumulate(statistics.begin(), statistics.end(), 0.0f)
			/ (T(statistics.size()));
		std::cout << std::endl;
		std::cout << "## Average: length=" << xbar;
		std::cout << std::endl;
		T sigmabar = T(0);
		for (typename std::vector<T>::const_iterator x = statistics.begin(); x != statistics.end();
			++x)
			sigmabar += pow((*x - xbar), 2);
		sigmabar = sqrt(sigmabar) / T(statistics.size());
		T se/*standard error*/ = sigmabar / sqrt(T(statistics.size()));
		std::cout << "## (+- 95%) =" << (se * 2);
		std::cout << std::endl;
		statistics.clear();
#endif
	}


	void stateAction(){
		if (!agentAction)
		{
			/*Initialize the problem*/
			problem->initialize();

			/*Update the state variables*/
			problem->updateTuple();

			/*Statistic variables*/
			timeStep = 0;
			episodeR = 0;
			episodeZ = 0;
			totalTimeInMilliseconds = 0;
			/*The episode is just started*/
			endingOfEpisode = false;
			problem->getTRStep()->setForcedEndOfEpisode(endingOfEpisode);

			/*Initialize the control agent and get the first action*/
			agentAction = agent->initialize(problem->getTRStep());
		}
		else
		{
			/*Step through the problem*/
			problem->step(agentAction);
		}
	}


	int reward(){
		int rewardCount = 0;
		if (!agentAction)
		{
		}
		else
		{
			/*Update the state variables*/
			problem->updateTuple();

			TRStep<T>* step = problem->getTRStep();
			++timeStep;
			episodeR += step->r_tp1;
			episodeZ += step->z_tp1;
			endingOfEpisode = step->endOfEpisode || (timeStep == maxEpisodeTimeSteps);
			rewardCount += episodeR;
			//step->setForcedEndOfEpisode(endingOfEpisode);
#if !defined(EMBEDDED_MODE)
			timer.start();
#endif
			agentAction = agent->getAtp1(step);
#if !defined(EMBEDDED_MODE)
			timer.stop();
			totalTimeInMilliseconds += timer.getElapsedTimeInMilliSec();
#endif
		}

		if (endingOfEpisode/*The episode is just ended*/)
		{
#if !defined(EMBEDDED_MODE)
			if (verbose)
			{
				T averageTimePerStep = totalTimeInMilliseconds / timeStep;
				std::cout << "{" << nbEpisodeDone << " [" << timeStep << " (" << episodeR << ", "
					<< episodeZ << ", " << averageTimePerStep << ")]} ";
				//std::cout << ".";
				std::cout.flush();
			}

			if (enableStatistics)
				statistics.push_back(timeStep);
#endif
			++nbEpisodeDone;
			/*Set the initial marker*/
			agentAction = 0;
			// Fire the events
			for (typename std::vector<Event*>::iterator iter = onEpisodeEnd.begin();
				iter != onEpisodeEnd.end(); ++iter)
			{
				Event* e = *iter;
				e->nbTotalTimeSteps = timeStep;
				e->nbEpisodeDone = nbEpisodeDone;
				e->averageTimePerStep = (totalTimeInMilliseconds / timeStep);
				e->episodeR = episodeR;
				e->episodeZ = episodeZ;
				e->update();
			}
		}
		
		return rewardCount;
	}

	void step()
	{
		stateAction();

		reward();
	}

	void runEpisodes()
	{
		do
		{
			step();
		} while (nbEpisodeDone < nbEpisodes);
	}

	void runEvaluate(const int& nbEpisodes = 20, const int& nbRuns = 1)
	{
#if !defined(EMBEDDED_MODE)
		std::cout << "\n@@ Evaluate=" << enableTestEpisodesAfterEachRun << std::endl;
#endif
		RLAgent<T>* evaluateAgent = new ControlAgent<T>(agent->getRLAgent());
		RunnerOverride<T>* runner = new RunnerOverride<T>(evaluateAgent, problem, maxEpisodeTimeSteps,
			nbEpisodes, nbRuns);
		runner->run();
		delete evaluateAgent;
		delete runner;
	}

	void run()
	{
#if !defined(EMBEDDED_MODE)
		for (int run = 0; run < nbRuns; run++)
		{
			if (verbose)
				std::cout << "\n@@ Run=" << run << std::endl;
			if (enableStatistics)
				statistics.clear();
			nbEpisodeDone = 0;
			// For each run
			agent->reset();
			runEpisodes();
			if (enableStatistics)
				benchmark();

			if (enableTestEpisodesAfterEachRun)
				runEvaluate(maxTestEpisodesAfterEachRun);
		}
#endif
	}

	bool isBeginingOfEpisode() const
	{
		return agentAction == 0;
	}

	const Action<T>* getAgentAction() const
	{
		return agentAction;
	}

	bool isEndingOfEpisode() const
	{
		return endingOfEpisode;
	}

	bool isRunning() const
	{
		return (nbEpisodeDone < nbEpisodes) || (nbEpisodes == -1);
	}

	const RLProblem<T>* getRLProblem() const
	{
		return problem;
	}

	int getMaxEpisodeTimeSteps() const
	{
		return maxEpisodeTimeSteps;
	}

	void computeValueFunction(const char* outFile = "visualization/valueFunction.txt") const
	{
#if !defined(EMBEDDED_MODE)
		if (problem->dimension() == 2) // only for two state variables
		{
			std::ofstream out(outFile);
			PVector<T> x_t(2);
			for (T x = 0; x <= 10; x += 0.1f)
			{
				for (T y = 0; y <= 10; y += 0.1f)
				{
					x_t.at(0) = x;
					x_t.at(1) = y;
					out << agent->computeValueFunction(&x_t) << " ";
				}
				out << std::endl;
			}
			out.close();
		}

		// draw
		problem->draw();
#endif
	}
};
