/**
 * @file main.cpp
 * @brief Single thread control of the assembly
 * @author Benjamin Navarro
 * @version 1.0.0
 * @date 2015-10-12
 */

#include <iostream>
#include <thread>

#include "simulator.h"

using namespace std;

/**
 * @brief Start the assembly operations
 *
 * @param op operation to perform (1-3)
 */
void assembly(int op) {
	sim.set_G(true);
	sim.wait_pos_assem();
	sim.set_G(false);

	if(op==1) {
		sim.set_OP1(true);
		while(not sim.read_fin_OP1())
			this_thread::sleep_for(std::chrono::milliseconds(10));
		sim.set_OP1(false);
	}
	else if(op==2) {
		sim.set_OP2(true);
		while(not sim.read_fin_OP2())
			this_thread::sleep_for(std::chrono::milliseconds(10));
		sim.set_OP2(false);
	}
	else {
		sim.set_OP3(true);
		while(not sim.read_fin_OP3())
			this_thread::sleep_for(std::chrono::milliseconds(10));
		sim.set_OP3(false);
		cout << "Verif=1" << endl;
		sim.set_Verif(true);
		while(not sim.read_assemblage_conforme())
			this_thread::sleep_for(std::chrono::milliseconds(10));
		sim.set_Verif(false);
		cout << "Verif=0" << endl;
		while(not sim.read_assemblage_evacue())
			this_thread::sleep_for(std::chrono::milliseconds(10));
		cout << "Assembly evacuated" << endl;
	}
	sim.set_D(true);
	sim.wait_pos_t1();
	sim.set_D(false);
}

/**
 * @brief Evacuate the object
 */
void evac() {
	sim.set_D(true);
	sim.wait_pos_t2();
	sim.set_D(false);

	sim.set_AV_T2(false);
	sim.wait_arret_t2();

	sim.set_Pose(true);
	sim.wait_fpose();
	sim.set_Pose(false);

	sim.set_AV_T2(true);

	sim.set_G(true);
	sim.wait_pos_t1();
	sim.set_G(false);
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
	if(not sim.start(10)) {
		return -1;
	}

	int needed = 1;
	int loops = 0;

	cout << "D=1" << endl;
	sim.set_D(true);
	sim.wait_pos_t1();
	sim.set_D(false);
	cout << "D=0" << endl;

	while(loops < 2) {
		cout << "AV_T1=1" << endl;
		sim.set_AV_T1(true);
		sim.wait_co();
		sim.set_AV_T1(false);
		cout << "AV_T1=0" << endl;

		cout << "Reccam=1" << endl;
		sim.set_Reccam(true);
		while(not sim.read_fin_reccam())
			this_thread::sleep_for(std::chrono::milliseconds(10));
		sim.set_Reccam(false);
		cout << "Reccam=0" << endl;

		cout << "Prend=1" << endl;
		sim.set_Prend(true);
		sim.wait_fprise();
		sim.set_Prend(false);
		cout << "Prend=0" << endl;

		cout << "P1=" << sim.read_p1() << ", P2=" << sim.read_p2() << ", P3=" << sim.read_p3() << endl;

		if(needed==1 and sim.read_p1()) {
			cout << "assembly op1" << endl;
			assembly(1);
			needed = 2;
		}
		else if(needed==2 and sim.read_p2()) {
			cout << "assembly op2" << endl;
			assembly(2);
			needed = 3;
		}
		else if(needed==3 and sim.read_p3()) {
			cout << "assembly op3" << endl;
			assembly(3);
			needed = 1;
			loops = loops+1;
		}
		else {
			cout << "evac" << endl;
			evac();
		}
	}

	sim.stop();

	return 0;
}
