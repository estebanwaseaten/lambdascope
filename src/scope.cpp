// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */

 #include "scope.h"

#include "adc_MCP3201.h"
#include "adc_dummy.h"

#include <cstring>

scope::scope( int channels )
{
    cout << "scope::scope()" << endl;
    className = "scope";

    channelCount = channels;
    scopeChannels = new channel[channelCount];
    averaging = 1;

    myRaspi = new raspi_info();                 //create system info first
    mainADC = new adc_MCP3201( myRaspi );       //pass system info to ADC if they need it
    //mainADC->setSpeed(5000000);
	//mainADC = new adc_dummy();

    startup = high_resolution_clock::now();
}


scope::~scope()
{
    cout << "scope::~scope()" << endl;

    delete myRaspi;
    delete[] scopeChannels;
    delete mainADC;
}

void scope::setAveraging( uint32_t avg )
{
    if( averaging != avg )
    {
        averaging = avg;
        cout << "set averaging to" << averaging << endl;
    }
}

//acquire data and add it to channel buffer --> slow this down for longer timespan acquisition!!!
void scope::looped()
{
    double  dataAvg[channelCount];
    double  timeAvg = 0;

    //set averaging accumulators to zero for each channel:
    for (size_t i = 0; i < channelCount; i++)
    {
        dataAvg[i] = 0;
    }

    //average:
    for (size_t j = 0; j < averaging; j++)
    {
        mainADC->acquireData();                      //1. acquire data - could be for more than one channel

        now = high_resolution_clock::now();
        microseconds duration = duration_cast<microseconds>(now - startup);

        for (size_t i = 0; i < channelCount; i++)
        {
            dataAvg[i] += mainADC->getData(i);      //2. fetch acquired data for each channel
        }
        timeAvg += duration.count();
    }

    //write output data into each scopeChannel
    for (size_t i = 0; i < channelCount; i++)
    {
        scopeChannels[i].addDataPoint( dataAvg[i]/averaging, timeAvg/averaging );
    }
}

// copies data from channel buffers into display buffers (data and time)
// fills the complete buffers data and time with numPoints values.
bool scope::collectDataUntilFull( uint16_t *data, uint32_t *time, uint32_t numPoints, uint8_t channel )
{
    scopeChannels[channel].collectDataUntilFull( data, time, numPoints );
}
