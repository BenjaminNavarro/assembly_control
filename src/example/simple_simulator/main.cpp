/**
 * @file main.cpp
 * @brief Single thread control of the assembly (simplified version)
 * @author Benjamin Navarro
 * @version 1.0.0
 * @date 2015-10-12
 */

#include <iostream>
#include <thread>

#include "simple_simulator.h"

using namespace std;

/**
 * @brief Start the assembly operations
 *
 * @param op operation to perform (1-3)
 */
void assembly() {
    simple_sim.set_G(true);
    simple_sim.wait_pos_assem();
    simple_sim.set_G(false);
    
	simple_sim.set_OP(true); // Blocking call
	simple_sim.set_OP(false);
    
	simple_sim.set_D(true);
    simple_sim.wait_pos_t1();
    simple_sim.set_D(false);
}
        
/**
 * @brief Evacuate the object
 */
void evac() {
    simple_sim.set_D(true);
    simple_sim.wait_pos_t2();
    simple_sim.set_D(false);
    
    simple_sim.set_AV_T2(false);
    simple_sim.wait_arret_t2();
    
    simple_sim.set_Pose(true); // Blocking call
    simple_sim.set_Pose(false);
    
    simple_sim.set_AV_T2(true);
    
    simple_sim.set_G(true);
    simple_sim.wait_pos_t1();
    simple_sim.set_G(false);
}

/**
 * @brief Main function, control the assembly process
 *
 * @param argc Not used
 * @param argv[] Not used
 *
 * @return -1 in case of an error, 0 otherwise
 */
int main(int argc, char const *argv[])
{
    if(not simple_sim.start(10)) {
    	return -1;
    }
    
    int op = 0;
            
    cout << "D=1" << endl;
    simple_sim.set_D(true);
    simple_sim.wait_pos_t1();
    simple_sim.set_D(false);
    cout << "D=0" << endl;
    
    while(op < 6) {
        cout << "AV_T1=1" << endl;
        simple_sim.set_AV_T1(true);
        simple_sim.wait_co();
        simple_sim.set_AV_T1(false);
        cout << "AV_T1=0" << endl;
    
        cout << "Reccam=1" << endl;
        simple_sim.set_Reccam(true);
        while(not simple_sim.read_fin_reccam())
            this_thread::sleep_for(std::chrono::milliseconds(10));
        simple_sim.set_Reccam(false);
        cout << "Reccam=0" << endl;
    
        cout << "Prend=1" << endl;
        simple_sim.set_Prend(true); // Blocking call
        simple_sim.set_Prend(false);
        cout << "Prend=0" << endl;
  
        if(simple_sim.read_assembler()) {
            cout << "assembly op" << endl;
            assembly();
			++op;
		}
        else if(simple_sim.read_evacuer()) {
            cout << "evac" << endl;
            evac();
		}
		else {
			cout << "Error, no assembly or evacuation signal" << endl;
			break;
		}
	}
            
    simple_sim.stop();

	return 0;
}
