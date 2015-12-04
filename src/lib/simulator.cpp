/**
 * @file simulator.cpp
 * @brief Simulator class implementation
 * @author Benjamin Navarro
 * @version 1.0.0
 * @date 2015-10-12
 */

#include "simulator.h"

#include <iostream>
#include <chrono>
#include <thread>

#include <cstring>
#include <time.h>
#include <sys/time.h>

extern "C" {
    #include "extApi.h"
}

using namespace std;

#define LAZY_MODE 0

Simulator::Simulator() : 
	client_id_(-1),
	run_(false),
	prev_position_(PosAssembly),
	prev_gripper_state_(0),
	prev_optical_barrier_state_(0),
	prev_evac_conveyor_state_(1),
	last_created_object_time_(0),
	last_created_object_type_(0)
{
	memset(&commands_, 0, sizeof(commands_t));
	
	set_AV_T1(true);
	set_AV_T2(true);
}

Simulator::~Simulator() {
	
}

bool Simulator::start(int cycle_ms) {
	client_id_ = simxStart((simxChar*)"127.0.0.1",19997,true,true,2000,5);
	if (client_id_ != -1) {
		cout << "Connected to V-REP" << endl;

		if(not get_Handles()) {
			cout << "Failed to get object handles" << endl;
			return false;
		}

		if(not start_Streaming()) {
			cout << "Starting streaming values failed" << endl;
			return false;
		}

		simxStartSimulation(client_id_, simx_opmode_oneshot_wait);

		cout << "Simulation started" << endl;
		
		this_thread::sleep_for(std::chrono::seconds(1));

		run_ = true;
		thread_ = thread(&Simulator::process, this, cycle_ms);
	}
	else {
		cerr << "Can't connect to V-REP" << endl;
		simxFinish(client_id_);
		return false;
	}

	return true;
}

void Simulator::stop() {
	if(run_) {
		run_ = false;
		thread_.join();	
	}

	if(client_id_ >= 0) {
		simxStopSimulation(client_id_, simx_opmode_oneshot_wait);
		simxFinish(client_id_);
		cout << "Simulation ended" << endl;
	}
}

bool Simulator::get_Handles() {
	int ret_code = simxGetObjectHandle(client_id_, "appro_proximity_sensor#", &appro_prox_sensor_handle_, simx_opmode_oneshot_wait);
	if(ret_code != simx_return_ok) {
		cout << "can't get appro_proximity_sensor handle (error " << ret_code << ")" << endl;
		return false;
	}

	return true;
}

bool Simulator::start_Streaming() {
	simxInt tmp;
	bool all_ok = true;
	
	all_ok &= ((simxGetIntegerSignal(client_id_, "optical_barrier_state",	&tmp, simx_opmode_streaming) & 0xFE) == 0);
	all_ok &= ((simxGetIntegerSignal(client_id_, "gripper_closed", 			&tmp, simx_opmode_streaming) & 0xFE) == 0);
	all_ok &= ((simxGetIntegerSignal(client_id_, "current_position", 		&tmp, simx_opmode_streaming) & 0xFE) == 0);
	all_ok &= ((simxGetIntegerSignal(client_id_, "evac_conveyor_stopped", 	&tmp, simx_opmode_streaming) & 0xFE) == 0);

	all_ok &= ((simxGetIntegerSignal(client_id_, "end_identification", 		&tmp, simx_opmode_streaming) & 0xFE) == 0);
	all_ok &= ((simxGetIntegerSignal(client_id_, "box_type", 				&tmp, simx_opmode_streaming) & 0xFE) == 0);
	all_ok &= ((simxGetIntegerSignal(client_id_, "end_operation", 			&tmp, simx_opmode_streaming) & 0xFE) == 0);
	all_ok &= ((simxGetIntegerSignal(client_id_, "assembly_ok",		 		&tmp, simx_opmode_streaming) & 0xFE) == 0);
	all_ok &= ((simxGetIntegerSignal(client_id_, "assembly_evacuated", 		&tmp, simx_opmode_streaming) & 0xFE) == 0);
	
	return all_ok;
}

double Simulator::get_Current_Time() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return double(tv.tv_sec + tv.tv_usec*1e-6);
}

void Simulator::process(int cycle_ms) {
	typedef chrono::duration<int, chrono::milliseconds::period> cycle;
	
	simxInt optical_barrier_state, gripper_state, position, evac_conveyor_stopped;
	simxInt end_identification, box_type, end_operation, assembly_ok, assembly_evacuated;	
	
	simxGetIntegerSignal(client_id_, "optical_barrier_state",	&optical_barrier_state, 	simx_opmode_oneshot_wait);
	simxGetIntegerSignal(client_id_, "gripper_closed", 			&gripper_state, 			simx_opmode_oneshot_wait);
	simxGetIntegerSignal(client_id_, "current_position", 		&position, 					simx_opmode_oneshot_wait);
	simxGetIntegerSignal(client_id_, "evac_conveyor_stopped", 	&evac_conveyor_stopped, 	simx_opmode_oneshot_wait);

	simxGetIntegerSignal(client_id_, "end_identification", 		&end_identification, 		simx_opmode_oneshot_wait);
	simxGetIntegerSignal(client_id_, "box_type", 				&box_type, 					simx_opmode_oneshot_wait);
	simxGetIntegerSignal(client_id_, "end_operation", 			&end_operation, 			simx_opmode_oneshot_wait);
	simxGetIntegerSignal(client_id_, "assembly_ok", 			&assembly_ok, 				simx_opmode_oneshot_wait);
	simxGetIntegerSignal(client_id_, "assembly_evacuated", 		&assembly_evacuated, 		simx_opmode_oneshot_wait);		
	
	cout << "Simulator communication thread started. Cycle time = " << cycle_ms << "ms" << endl;
	
	while(run_) {
		auto start_time = chrono::steady_clock::now();
		auto end_time = start_time + cycle(cycle_ms);	
		
		/***********************		Signals			************************/
		simxGetIntegerSignal(client_id_, "optical_barrier_state",	&optical_barrier_state, 	simx_opmode_streaming);
		simxGetIntegerSignal(client_id_, "gripper_closed", 			&gripper_state, 			simx_opmode_streaming);
		simxGetIntegerSignal(client_id_, "current_position", 		&position, 					simx_opmode_streaming);
		simxGetIntegerSignal(client_id_, "evac_conveyor_stopped", 	&evac_conveyor_stopped, 	simx_opmode_streaming);
                                                                                                
		simxGetIntegerSignal(client_id_, "end_identification", 		&end_identification, 		simx_opmode_streaming);
		simxGetIntegerSignal(client_id_, "box_type", 				&box_type, 					simx_opmode_streaming);
		simxGetIntegerSignal(client_id_, "end_operation", 			&end_operation, 			simx_opmode_streaming);
		simxGetIntegerSignal(client_id_, "assembly_ok", 			&assembly_ok, 				simx_opmode_streaming);
		simxGetIntegerSignal(client_id_, "assembly_evacuated", 		&assembly_evacuated, 		simx_opmode_streaming);	
		
		if(optical_barrier_state != prev_optical_barrier_state_) {
			prev_optical_barrier_state_ = optical_barrier_state;
			
			if(optical_barrier_state)
				signals_.co.notify();
		}	

		if(gripper_state != prev_gripper_state_) {
			if(gripper_state)
				signals_.fprise.notify();
			else
				signals_.fpose.notify();
			prev_gripper_state_ = gripper_state;
		}

		if(position != prev_position_) {
			switch(position) {
				case PosAssembly:
					signals_.pos_assem.notify();
				break;
				case PosAppro:
					signals_.pos_t1.notify();
				break;
				case PosEvac:
					signals_.pos_t2.notify();
				break;
			}
			prev_position_ = Position(position);
			//cout << "position = " << position << endl;
		}

		if(evac_conveyor_stopped != prev_evac_conveyor_state_) {
			prev_evac_conveyor_state_ = evac_conveyor_stopped;
			
			if(evac_conveyor_stopped)
				signals_.arret_t2.notify();
		}
		
		signals_.fin_reccam = end_identification;
		signals_.p1 = (box_type == 1);
		signals_.p2 = (box_type == 2);
		signals_.p3 = (box_type == 3);
		signals_.fin_OP1 = (end_operation == 1);
		signals_.fin_OP2 = (end_operation == 2);
		signals_.fin_OP3 = (end_operation == 3);
		signals_.assemblage_conforme = assembly_ok;
		signals_.assemblage_evacue = assembly_evacuated;
		
		/*********************** 		Commands		***********************/
		static bool test_t1 = true;
		if(commands_.AV_T1) {
			simxSetIntegerSignal(client_id_, "appro_conveyor_command", 1, simx_opmode_oneshot);
			// Add new boxes to the conveyor
			if(test_t1) {
				last_created_object_time_ = get_Current_Time();
				test_t1 = false;
			}
			if((get_Current_Time() - last_created_object_time_) > 1.) {
				last_created_object_time_ = get_Current_Time();

#if LAZY_MODE
				static int type = 0;
				++type;
				if(type > 3)
					type = 1;
#else
				int object_to_add = rand() % 100;
				int type;
				
				if(last_created_object_type_ == 0) {
					if(object_to_add < 33)
						type = 1;
					else if(object_to_add < 66)
						type = 2;
					else
						type = 3;	
				}
				else {
					if(last_created_object_type_ == 1) {
						if(object_to_add < 60)
							type = 2;
						else 
							type = 3;
					}
					else if(last_created_object_type_ == 2) {
						if(object_to_add < 40)
							type = 1;
						else 
							type = 3;
					}
					else {
						if(object_to_add < 60)
							type = 1;
						else 
							type = 2;
					}
				}
				
				last_created_object_type_ = type;
#endif
					
				simxSetIntegerSignal(client_id_, "add_object", type, simx_opmode_oneshot);
			}
		}
		else {
			simxSetIntegerSignal(client_id_, "appro_conveyor_command", 0, simx_opmode_oneshot);
			test_t1 = true;
		}
		
		if(commands_.AV_T2)
			simxSetIntegerSignal(client_id_, "evac_conveyor_command", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "evac_conveyor_command", 0, simx_opmode_oneshot);
		
		if(commands_.Reccam)
			simxSetIntegerSignal(client_id_, "reccam", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "reccam", 0, simx_opmode_oneshot);
		
		if(commands_.D)		
			simxSetIntegerSignal(client_id_, "go_right", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "go_right", 0, simx_opmode_oneshot);
		
		if(commands_.G)		
			simxSetIntegerSignal(client_id_, "go_left", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "go_left", 0, simx_opmode_oneshot);		
		
		if(commands_.Prend)		
			simxSetIntegerSignal(client_id_, "take", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "take", 0, simx_opmode_oneshot);	
		
		if(commands_.Pose)		
			simxSetIntegerSignal(client_id_, "put_down", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "put_down", 0, simx_opmode_oneshot);	
		
		if(commands_.OP1)		
			simxSetIntegerSignal(client_id_, "OP1", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "OP1", 0, simx_opmode_oneshot);	
		
		if(commands_.OP2)		
			simxSetIntegerSignal(client_id_, "OP2", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "OP2", 0, simx_opmode_oneshot);	
		
		if(commands_.OP3)		
			simxSetIntegerSignal(client_id_, "OP3", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "OP3", 0, simx_opmode_oneshot);	
		
		if(commands_.Verif)		
			simxSetIntegerSignal(client_id_, "verif", 1, simx_opmode_oneshot);
		else
			simxSetIntegerSignal(client_id_, "verif", 0, simx_opmode_oneshot);	

		this_thread::sleep_until(end_time);
	}
}

bool Simulator::wait_co(int msec) {
	if(msec < 0) {
		signals_.co.wait();
		return true;
	}
	else
		return signals_.co.wait_for(msec);
}

bool Simulator::wait_fprise(int msec) {
	if(msec < 0) {
		signals_.fprise.wait();
		return true;
	}
	else
		return signals_.fprise.wait_for(msec);
}

bool Simulator::wait_fpose(int msec) {
	if(msec < 0) {
		signals_.fpose.wait();
		return true;
	}
	else
		return signals_.fpose.wait_for(msec);
}

bool Simulator::wait_pos_t1(int msec) {
	if(msec < 0) {
		signals_.pos_t1.wait();
		return true;
	}
	else
		return signals_.pos_t1.wait_for(msec);
}

bool Simulator::wait_pos_t2(int msec) {
	if(msec < 0) {
		signals_.pos_t2.wait();
		return true;
	}
	else
		return signals_.pos_t2.wait_for(msec);
}

bool Simulator::wait_pos_assem(int msec) {
	if(msec < 0) {
		signals_.pos_assem.wait();
		return true;
	}
	else
		return signals_.pos_assem.wait_for(msec);
}

bool Simulator::wait_arret_t2(int msec) {
	if(msec < 0) {
		signals_.arret_t2.wait();
		return true;
	}
	else
		return signals_.arret_t2.wait_for(msec);
}

bool Simulator::read_fin_reccam() {
	return signals_.fin_reccam;
}

bool Simulator::read_p1() {
	return signals_.p1;
}

bool Simulator::read_p2() {
	return signals_.p2;
}

bool Simulator::read_p3() {
	return signals_.p3;
}

bool Simulator::read_fin_OP1() {
	return signals_.fin_OP1;
}

bool Simulator::read_fin_OP2() {
	return signals_.fin_OP2;
}

bool Simulator::read_fin_OP3() {
	return signals_.fin_OP3;
}

bool Simulator::read_assemblage_conforme() {
	return signals_.assemblage_conforme;
}

bool Simulator::read_assemblage_evacue() {
	return signals_.assemblage_evacue;
}


/***	Commands	***/
void Simulator::set_AV_T1(bool state) {
	commands_.AV_T1 = state;
}

void Simulator::set_AV_T2(bool state) {
	commands_.AV_T2 = state;
}

void Simulator::set_Reccam(bool state) {
	commands_.Reccam = state;
}

void Simulator::set_D(bool state) {
	commands_.D = state;
}

void Simulator::set_G(bool state) {
	commands_.G = state;
}

void Simulator::set_Prend(bool state) {
	commands_.Prend = state;
}

void Simulator::set_Pose(bool state) {
	commands_.Pose = state;
}

void Simulator::set_OP1(bool state) {
	commands_.OP1 = state;
}

void Simulator::set_OP2(bool state) {
	commands_.OP2 = state;
}

void Simulator::set_OP3(bool state) {
	commands_.OP3 = state;
}

void Simulator::set_Verif(bool state) {
	commands_.Verif = state;
}

Simulator sim;
