/**
 * @file task_example.cpp
 * @brief Example of two tasks running in separate threads
 * @author Benjamin Navarro
 * @version 1.0.0
 * @date 2015-10-12
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>

#include "simulator.h"

StatusSignal signal_1, signal_2;

enum EtatsTache {
	etat1,
	etat2,
	etat3
};

void tache() {
	typedef std::chrono::duration<int, std::chrono::milliseconds::period> cycle;  //define the type 'cycle'

	EtatsTache etat = etat1;

	while(1) {
		auto start_time = std::chrono::steady_clock::now();             // start_time = current time
		auto end_time = start_time + cycle(100);                        // end_time = current_timme + 10ms

		std::cout << ".";
		std::flush(std::cout); // Force the text to be printed in the console

		switch(etat) {
		case etat1:
			if(signal_1.is_set()) {
				signal_1.clear();
				etat = etat2;
			}
			else if(signal_2.is_set()) {
				signal_2.clear();
				etat = etat3;
			}
			break;

		case etat2:
			std::cout << "etat2" << std::endl;
			etat = etat1;
			break;

		case etat3:
			std::cout << "etat3" << std::endl;
			return; // just to quit the program
			break;
		}
		std::this_thread::sleep_until(end_time);                             // Stop thread execution until 'end_time' (next cycle)
	}
}

int main(int argc, char const *argv[])
{
	std::thread ctrl_tache;

	ctrl_tache = std::thread(tache);

	std::this_thread::sleep_for(std::chrono::duration<int, std::chrono::seconds::period>(2));

	// Signal an event to the thread
	signal_1.set();

	std::this_thread::sleep_for(std::chrono::duration<int, std::chrono::seconds::period>(2));

	// Signal an event to the thread
	signal_2.set();

	// Wait for thread completion before exiting
	ctrl_tache.join();

	return 0;
}
