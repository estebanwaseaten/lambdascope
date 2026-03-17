// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */
#include <iostream>
#include <thread>

#include <sys/syscall.h>
#include <unistd.h>

#include "lambdascope.h"
#include "controller.h"

using namespace std;

int main( void )
{
	cout << "lambdascope 0.0.1" << endl;
	cout << "hardware threads: " << thread::hardware_concurrency() << std::endl;

	controller *mainController = new controller();
	mainController->mainLoop();

	//cout << "exit main program, PID: " << syscall(SYS_gettid) << endl;
	//threads are joined in destructor of threads class!
	delete mainController;

	return 0;
}
