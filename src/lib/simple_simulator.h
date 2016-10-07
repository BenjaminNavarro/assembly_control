/**
 * @file simple_simulator.h
 * @brief A simplifed version of the Simulator class
 * @author Benjamin Navarro
 * @version 1.0.0
 * @date 2015-10-12
 */

#ifndef SIMPLE_SIMULATOR_H_
#define SIMPLE_SIMULATOR_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include "simulator.h"

/**
 * @brief Simplified interface with the V-REP simulator
 */
class SimpleSimulator : public Simulator
{
public:
	SimpleSimulator();
	~SimpleSimulator();

	SimpleSimulator(const SimpleSimulator& sim) {
	}

	SimpleSimulator& operator=(const SimpleSimulator& sim) {
		return *this;
	}

	/***	Signals		***/
	/**
	 * @brief Read the assembler signal (assembly needed)
	 *
	 * @return State of the signal
	 */
	bool read_assembler();
	/**
	 * @brief Read the evacuer signal (evacuation needed)
	 *
	 * @return State of the signal
	 */
	bool read_evacuer();

	/***	Commands	***/
	/**
	 * @brief Set the Prend command (take an object) and wait for its completion
	 *
	 * @param state Set (true) or reset (false) the command
	 */
	virtual void set_Prend(bool state);
	/**
	 * @brief Set the Pose command (put down an object) and wait for its completion
	 *
	 * @param state
	 */
	virtual void set_Pose(bool state);
	/**
	 * @brief Set the OP command (assembly operation) and wait for its completion
	 *
	 * @param state
	 */
	void set_OP(bool state);

private:
	int next_OP_;
	int wanted_object_;
};

extern SimpleSimulator simple_sim;

#endif
