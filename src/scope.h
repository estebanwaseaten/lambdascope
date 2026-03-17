// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */

#ifndef scope_h
#define scope_h

#include <threader/threader.h>
#include <raspi/raspi.h>

//#include "adc.h"
#include "channel.h"
//#include "custom_elements.h"

//#define BUFFER_SIZE 1024 //maybe make this more flexible

class scope : public threader
{
public:
    scope( int channels );
    ~scope();

    bool collectDataUntilFull( uint16_t *data, uint32_t *time, uint32_t numPoints, uint8_t channel );
    void setAveraging( uint32_t );
    void fetchDataVolt( float *storage );
    void fetchNewDataVolt( float *storage );
    bool isNewDataAvailable();


private:
    void looped() override;



    int       daqDelay = 0;
    int       channelCount;
    channel   *scopeChannels;

    uint32_t   averaging = 1;

    raspi_info  *myRaspi;
    adc         *mainADC;                 //could interface with more than one?

/*
    uint32_t    bufferSize = 1024;

    //raw data
    uint16_t    *rawBuffer1;
    uint16_t    *rawBuffer2;

    //raw time info
    uint32_t    *timeBuffer1;
    uint32_t    *timeBuffer2;

    uint32_t    buffer_index = 0;

    uint16_t    *rawBufferWrite;        //points to one of the rawBuffers above
    uint16_t    *rawBufferRead;

    uint32_t    *timeBufferWrite;       //points to one of the timeBuffers above
    uint32_t    *timeBufferRead;

    uint32_t    readBufferStartTime;
    uint32_t    readBufferStopTime;

    bool readOutInProgress = false;
    bool newDataAvailable = false;*/

    std::chrono::time_point<std::chrono::high_resolution_clock> startup, now;

};




#endif
