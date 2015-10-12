/**
 * @file main.cpp
 * @brief 
 * @author 
 * @version 1.0.0
 * @date 2015-10-12
 */

#include <iostream>
#include <thread>

#include "simulator.h"

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
    if(not sim.start(10)) {
    	return -1;
    }
    
    sim.stop();

	return 0;
}
