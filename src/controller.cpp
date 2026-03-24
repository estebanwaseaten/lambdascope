// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */

#include "controller.h"
//#include "utilities.h"
#include "debug.h"

#include <chrono>
#include <thread>
#include <functional>
#include <cmath>



using namespace std;
using namespace std::chrono;
using namespace std::placeholders;

controller::controller()
{
	cout << "controller::controller()" << endl;

	//oscilloscope controller (handles channels (data) and acquisition (adc))
	//mainScope = new scope( 1 );
	//mainScope->setDelay(10);
	//exit here for debug:
	//abort();

	myRaspi = new raspi_info();
	myRaspi->spidev_set_handle( SPI_HANDLE_B );

	//screen to draw to
	mainCanvas = new canvas();

	//terminalio
	mainTerminal = new terminal();						// if input is handled --> loop is stopped
	mainInput = mainTerminal->createMainInput();		//will be deleted when mainTerminal is deleted
	mainConsole = mainTerminal->createMainConsole();	//will be deleted when mainTerminal is deleted
	// register event handlers for input:
	mainInput->setKeyboardInputSimpleCallback( bind(&controller::keyboardInput, this, _1, _2 ) );
	mainInput->setKeyboardInputSpecialCallback( bind(&controller::keyboardInputSpecial, this, _1, _2 ) );
	mainInput->setMouseDownCallback( bind(&controller::mouseDown, this, _1, _2, _3 ) );
	mainInput->setMouseUpCallback( bind(&controller::mouseUp, this, _1, _2, _3 ) );
	mainInput->setMouseScrollCallback( bind(&controller::mouseScroll, this, _1, _2, _3 ) );
	mainInput->setActiveCallback( bind(&controller::stillActive, this ) );
	mainInput->setDelay(10);
	mainInput->setVerbose( true );

	//start all thread loops:
	//mainScope->setDelay(1);
	//mainScope->start(123);



	mainInput->start(2);



	//for testing:
	mainCanvas->addElement( &myElement );				//myElement and myScopeElement cannot!!! be deleted before canvas is... --> make this safer? how??
	mainCanvas->addElement( &myBoxElement );				//myProgressBarElement should be drawn first...

	myElement.addChild( &myScopeElement );
	myBoxElement.addChild( &myProgressBarElement );
	myProgressBarElement.setPercent( 67 );
	mainCanvas->drawLine( 13, 10, 1, 1);
}

controller::~controller()
{
	cout << "controller::~controller()" << endl;
	cout << "counter: " << counter << endl;

	//mainConsole->stop();
	//spInterface->stop();		//segmentation fault
	mainInput->stop();
	//mainScope->stop();
	cout << "~controller(): all threads stopped!" << endl << endl;

	//threads are joined in destructor of threads class
	delete mainTerminal;
	delete mainCanvas;	//seg fault!
	//delete mainScope;
	delete myRaspi;
}

uint16_t controller::doubleTransfer( uint16_t cmd )
{
	simpleTransfer( cmd );
	return simpleTransfer( 0x5000 );
}

uint16_t controller::simpleTransfer( uint16_t cmd )
{
	uint8_t transfer[2] = {(uint8_t)(cmd >> 8), (uint8_t)cmd };
	myRaspi->spiTransfer( transfer, 2 );

	uint16_t highbits = transfer[0];
    uint16_t lowbits = transfer[1];
    //uint16_t result = (highbits << 8) + lowbits;
	return (highbits << 8) + lowbits;
}

void controller::fetchData( uint32_t channel, uint32_t size )
{
	uint16_t result;
	uint8_t *result8 = (uint8_t*)&result;		//returned data is in result8[0]

	for( int i = 0; i < size; i++ )
	{
		result = simpleTransfer( 0x5000 );
		cout << dec << (int)result << "; ";
	}
	cout << endl;
}

void controller::mainLoop()
{
	std::chrono::milliseconds idleTime(100);

	running = true;


	uint16_t result;
	uint8_t *result8 = (uint8_t*)&result;		//returned data is in result8[0]

	while( running )
	{
		if( !daq_paused )
		{
			result = doubleTransfer( 0x6000 );
			cout << hex << "0x" << (uint16_t)result8[1] << ", 0x" << (uint16_t)result8[0] << endl;

			if( result8[0] == 1 )
			{
				cout << "data available" << endl;
				result = doubleTransfer( 0x6201 );	//result is the response of the request.
				//the next data sent will be the size:
				result = simpleTransfer( 0x5000 );

				cout << "data size: " << dec << result << endl;

				fetchData( 1, result );		//channel, size

				//restart acquiring (should not be necessary after all data is transferred, but why not...)
				result = doubleTransfer( 0x5F00 );	//abort
				result = doubleTransfer( 0x6100 );	//start
			}

			//copy data from scope channel buffer to display buffer until display buffer is filled!!!
			//mainScope->setAveraging( myScopeElement.getAveragingRequest() );
			//mainScope->collectDataUntilFull( myScopeElement.getDataPtr(), myScopeElement.getTimePtr(), myScopeElement.getBufferSize(), 0 );
			//second channel: mainScope->copyData( myScopeElement.getDataPtr(), myScopeElement.getTimePtr(), myScopeElement.getBufferSize(), 1 );			//--> hwat to do if there is no data?	//--> hwat to do if there is no data?
			myScopeElement.process();
		}
		mainCanvas->draw();

		//just in case we want to do something when the terminal is resized --> not necessary
		int w, h;
		if (mainTerminal->didResizeWithReset( &w, &h ))	//check if resize happened... not very elegant
		{
			//get new dimensions and change buffers etc
			//DEBUG_MSG( "resize() manual check: w=" << w << " h;" << h );
		}

		std::this_thread::sleep_for(idleTime);
		counter++;
	}
}

//not necessary to use the callback
void controller::winResize( int w, int h )
{
	//cout << "resize callback: x=" << w << " y=" << h << endl;
}

bool controller::stillActive()
{
	// wait until input is processed and then return:
	while ( processingInput )
	{
		sleep(50);
	}
	return running;
}

void controller::mouseDown( int x, int y, char mod )	//location
{
	 //DEBUG_MSG( "mouseDown: (" << x << "," << y << ") modifier: 0x" << hex << uppercase << (int)mod << dec );
}

void controller::mouseUp( int x, int y, char mod )		//location
{
	//DEBUG_MSG( "mouseUp: (" << x << "," << y << ") modifier: 0x" << hex << uppercase << (int)mod << dec );
}

void controller::mouseScroll( int x, int y, char mod )	//how far scrolled?
{
	//DEBUG_MSG( "mouseScroll: (" << x << "," << y << ") modifier: 0x" << hex << uppercase << (int)mod << dec );
}

void controller::keyboardInputSpecial( char *buffer, int length )
{
	processingInput = true;	//maybe not necessary here...
	//first byte shoudl by 0x1B --> special character.

	uint16_t modlast;
	char  *modlastBytes = (char*)&modlast;
	modlastBytes[0] = buffer[length-2];		//modifier key none: 5B, shift 32, ctrl 35, alt 33, ctrl+alt 37
	modlastBytes[1] = buffer[length-1];		//keys:

	//cout << endl << getBinString(modlastBytes[1]) << " - " << getBinString(modlastBytes[0]) << endl;

	switch ( modlast )
	{
		case INPUT_ARROW_UP | INPUT_MOD_NONE:	//up arrow
			myScopeElement.decreaseVolt();
			break;
		case INPUT_ARROW_DOWN | INPUT_MOD_NONE:	//down arrow
			myScopeElement.increaseVolt();
			break;
		case INPUT_ARROW_RIGHT | INPUT_MOD_NONE:	//right arrow
			myScopeElement.increaseTimeRange();
			break;
		case INPUT_ARROW_LEFT | INPUT_MOD_NONE:	//left arrow
			myScopeElement.decreaseTimeRange();
			break;

		case INPUT_ARROW_UP | INPUT_MOD_ALT:
			myScopeElement.increaseTrigV(0.1);
			break;
		case INPUT_ARROW_DOWN | INPUT_MOD_ALT:
			myScopeElement.decreaseTrigV(0.1);
			break;
		case INPUT_ARROW_RIGHT | INPUT_MOD_ALT:
			myScopeElement.increaseTrigOffset();
			break;
		case INPUT_ARROW_LEFT | INPUT_MOD_ALT:
			myScopeElement.decreaseTrigOffset();
			break;
		case INPUT_ARROW_UP | INPUT_MOD_SHIFT:
			myScopeElement.decreaseVoltRange();
			break;
		case INPUT_ARROW_DOWN | INPUT_MOD_SHIFT:
			myScopeElement.increaseVoltRange();
			break;
	}

	processingInput = false;
}

//if false is returned --> tell the input to stop looping!
void controller::keyboardInput( char *buffer, int length )
{
	// tells stillActive() to wait until the Input is processed, before it returns!
	processingInput = true;
	char input = buffer[0];		//must be valid - some characters also have 2 bytes...

	int amount;

	time_point<std::chrono::high_resolution_clock> begin, end;
	duration<double> duration, percycle;

	switch( input )
	{
		case 'q':
			running = false;
			break;
		case 'w':
			{
				float buff[512];
				//spInterface->fetchNewDataVolt(buff);

				y0 = 2;
				//yZoom = 100;

				mainCanvas->clearBuffer();
				for( int i = 0; i < 512; i++ )
				{
					//cout << buff[i] << "; "; //*2.511/3100.0 << "; ";
					mainCanvas->drawPoint( i, (buff[i] - y0)*yZoom + 300 );
				}
				mainCanvas->swapBuffer();
				//cout << endl;
			}
			break;
		case 'r':
			//1. transfer data into scope element
			//spInterface->copyData( myScopeElement.getDataPtr(), myScopeElement.getTimePtr(), myScopeElement.getBufferSize() );			//--> hwat to do if there is no data?
			//2. process data there
			//myScopeElement.process();
			break;
	//	case 'i':
	//		mainCanvas->printInfo();
	//		break;
		case 'a':
			mainCanvas->swapBuffer();
			break;
		case 'f':
			mainCanvas->fill( 255, 0, 0 );
			break;

		case 'p':
		//	mainConsole->clearScreen();
		//	mainCanvas->printPixelBackBuffer(60, 30);
			daq_paused = !daq_paused;

			break;
		case 'c':
			myScopeElement.clearSweeps();
			break;
		case 'm':
			amount = 1000;
			begin = high_resolution_clock::now();
			for( int i = 0; i < amount; i++ )
			{
				//mainCanvas->displayRnd();
				//mainCanvas->swapBuffer();

				mainCanvas->fill( 255, 0, 255 );
			}
			end = high_resolution_clock::now();
			duration = duration_cast<nanoseconds>(end - begin);
			percycle = duration / amount;
			//counter = decimalConverter( &percycle );

			DEBUG_MSG( amount << " repeats duration: " << duration.count() << " s; single duration: " << percycle.count() );

			//mainCanvas->clear();
			break;
		case 'F':
			mainConsole->frame();
			break;
		case 't':
			myScopeElement.toggleTriggerMode();
			break;
		case 'g':
			myScopeElement.triggerSmoothingIncrease();
			break;
		case 'b':
			myScopeElement.triggerSmoothingDecrease();
			break;
		case 'd':
			//cout << spInterface->getDataVolt_ioctl() << endl;
			break;
		case 'i':
			scopeDelay++;
			//mainScope->setDelay_us(scopeDelay);
			//myScopeElement.increaseEvery();
			break;
		case 'k':
			if (scopeDelay!=0) {
				scopeDelay--;
			}
			//mainScope->setDelay_us(scopeDelay);
			//myScopeElement.decreaseEvery();
			break;
		case 'B':
			{
				//benchmarking different things
				cout << "Benchmark: " << endl;
				amount = 10000;
				begin = high_resolution_clock::now();
				for( int i = 0; i < amount; i++ )
				{
						//mainConsole->clearScreen();
						//spInterface->getDataVolt_ioctl();
						mainCanvas->drawElements();
				}
				end = high_resolution_clock::now();
				duration = duration_cast<nanoseconds>(end - begin);

				percycle = duration / amount;
				DEBUG_MSG( amount << " repeats duration: " << duration.count() << " s; single duration: " << percycle.count() << " rate: " << (float)amount/duration.count() << " Hz");

			}
			break;
		case 13:	//enter key
			//DEBUG_MSG("pressed enter!");
			cout << inputBuffer << endl;
			inputBuffer.clear();
			break;

		default:		//add to input Buffer and execute when pressing enter
			//DEBUG_MSG("echo " << hex << input);	//cout only outputs with << endl in the end!!
			//cout << hex << input << endl;
			inputBuffer += input;
			break;
	}

	processingInput = false;
}
