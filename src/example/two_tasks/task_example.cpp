/**
 * @file task_example.cpp
 * @brief Example of two tasks running in separate threads
 * @author Benjamin Navarro
 * @version 1.0.0
 * @date 2015-10-12
 */

#include <iostream>
#include <thread>
#include <unistd.h>

#include "simulator.h"

using namespace std;

enum EtatsTapis1 {
	Tapis1EnMarche,
	Tapis1Arrete
};

Signal demande_demarrage_tapis1;

void tache_tapis1() {
	EtatsTapis1 etat = Tapis1EnMarche;
	while(1) {
		switch(etat) {
		case Tapis1EnMarche:
			sim.set_AV_T1(true);
			sim.wait_co();                          // blocking call
			etat = Tapis1Arrete;
			break;

		case Tapis1Arrete:
			sim.set_AV_T1(false);
			if(demande_demarrage_tapis1.wait_for(10))       // blocking call (for 10ms)
				etat = Tapis1EnMarche;
			break;
		}
	}
}

enum EtatsAutreTache {
	etat1,
	etat2
};

void autre_tache() {
	typedef chrono::duration<int, chrono::milliseconds::period> cycle;  //define the type 'cycle'

	EtatsAutreTache etat = etat1;

	while(1) {
		auto start_time = chrono::steady_clock::now();                  // start_time = current time
		auto end_time = start_time + cycle(10);                         // end_time = current_timme + 10ms

		switch(etat) {
		case etat1:
			if(sim.read_fin_reccam())                                   // read signal, non-blocking call
				etat = etat2;
			break;

		case etat2:
			demande_demarrage_tapis1.notify();                          // Unlock tasks waiting on this signal
			break;
		}
		this_thread::sleep_until(end_time);                             // Stop thread execution until 'end_time' (next cycle)
	}
}

int main(int argc, char const *argv[])
{
	thread ctrl_tapis1;
	// May declare other threads here

	string cmd;
	while(1) {
		cout << "Enter init or end" << endl;
		cin >> cmd;
		if(cmd == "init") {
			if(not sim.start(10)) {
				return -1;
			}

			ctrl_tapis1 = thread(tache_tapis1);
			// May create other threads here
		}
		else if(cmd == "end") {
			sim.stop();
			break;
		}
	}

	return 0;
}
