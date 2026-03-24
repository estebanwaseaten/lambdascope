// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */


#ifndef controller_h
#define controller_h

#include <iostream>

//own libraries
#include <terminalio/terminal.h>
#include <terminalio/input.h>
#include <terminalio/console.h>

#include <canvas/canvas.h>

#include <raspi/raspi.h>

//this project
//#include "scope.h"
#include "custom_elements.h"

using namespace std;

class terminal;
class console;
class input;


class controller
{
public:
    controller();
    ~controller();

    void mainLoop();

    void createChannels( int count );

    bool stillActive();

    void keyboardInput( char *buffer, int length );
    void keyboardInputSpecial( char *buffer, int length );

    void mouseDown( int x, int y, char mod );
    void mouseUp( int x, int y, char mod );
    void mouseScroll( int x, int y, char mod );

    void winResize( int w, int h );

    //SPI helper methods
    uint16_t doubleTransfer( uint16_t cmd );
    uint16_t simpleTransfer( uint16_t cmd );
    void fetchData( uint32_t channel, uint32_t size );


private:
    long	   counter = 0;

    canvas    *mainCanvas;          //interface to monitor connected to raspi

    terminal  *mainTerminal;        //terminalio terminal -> sets up the linux/... terminal
    console   *mainConsole;         //text output (via ssh)
    input     *mainInput;           //handles keyboard and mouse input within the terminal/console

    //scope     *mainScope;       //different scope channels reside here.
    raspi_info  *myRaspi;


    bool      running = true;           //tracks if program is still running
    bool      processingInput = false;  //tracks if program is processing input right nows
    string    inputBuffer;              //simple input buffer for chars


    //scope --> should go to scope element
    double      y0 = 0.0;
    double      yZoom = 100.0;

    double      x0 = 0.0;
    double      xZoom = 1.0;

    bool        daq_paused = false;

    uint8_t         scopeDelay = 0;




    //for testing:
	element      myElement;							//dummy element
	scopeElement myScopeElement = scopeElement( 5, 5, 640, 480 );
	progressBarElement myProgressBarElement = progressBarElement( 12, 502, 196, 16);
	boxElement myBoxElement = boxElement( 10, 500, 200, 20);
};


#endif
