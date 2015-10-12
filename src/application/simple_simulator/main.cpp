/**
 * @file main.cpp
 * @brief 
 * @author 
 * @version 1.0.0
 * @date 2015-10-12
 */

#include <iostream>
#include <thread>

#include "simple_simulator.h"

using namespace std;

/**
 * @brief
 *
 * @param argc
 * @param argv[]
 *
 * @return
 */
int main(int argc, char const *argv[])
{
    if(not simple_sim.start(10)) {
    	return -1;
    }
    
    simple_sim.stop();

	return 0;
}
