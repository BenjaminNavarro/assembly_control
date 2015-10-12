/**
 * @file simple_simulator.cpp
 * @brief Implementation of the SimpleSimulator class
 * @author Benjamin Navarro
 * @version 1.0.0
 * @date 2015-10-12
 */

#include "simple_simulator.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <cstring>
#include <time.h>
#include <sys/time.h>

extern "C" {
	#include "extApi.h"
}

using namespace std;

SimpleSimulator::SimpleSimulator() : 
	next_OP_(1),
	Simulator()
{
}

SimpleSimulator::~SimpleSimulator() {
}

bool SimpleSimulator::read_assembler() {
	if((next_OP_ == 1 and read_p1()) or
	   (next_OP_ == 2 and read_p2()) or
	   (next_OP_ == 3 and read_p3())) 
		return true;
	else
		return false;
}

bool SimpleSimulator::read_evacuer() {
		if((next_OP_ == 1 and read_p1()) or
		   (next_OP_ == 2 and read_p2()) or
		   (next_OP_ == 3 and read_p3())) 
				return false;
		else
				return true;

}

void SimpleSimulator::set_Prend(bool state) {
	Simulator::set_Prend(state);
	if(state)
		wait_fprise();
}

void SimpleSimulator::set_Pose(bool state) {
	Simulator::set_Pose(state);
	if(state)
		wait_fpose();
}

void SimpleSimulator::set_OP(bool state) {
	switch(next_OP_) {
		case 1:
			set_OP1(state);
			if(state) {
				while(not read_fin_OP1())
					this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			else
				next_OP_ = 2;
		break;
		case 2:
			set_OP2(state);
			if(state) {
				while(not read_fin_OP2())
					this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			else
				next_OP_ = 3;
		break;
		case 3:
			set_OP3(state);
			if(state) {
				while(not read_fin_OP3())
					this_thread::sleep_for(std::chrono::milliseconds(10));

				simple_sim.set_Verif(true);
		        while(not simple_sim.read_assemblage_conforme())
		            this_thread::sleep_for(std::chrono::milliseconds(10));

				simple_sim.set_Verif(false);
				while(not simple_sim.read_assemblage_evacue())
					this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			else
				next_OP_ = 1;
		break;
	}
}

SimpleSimulator simple_sim;
