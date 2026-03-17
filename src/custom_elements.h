// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */


#ifndef custom_elements_h
#define custom_elements_h

#define SCOPE_MAX_VOLT_RANGE 50
#define SCOPE_MIN_VOLT_RANGE 0.01

#define SCOPE_MAX_TIME_RANGE 1000000
#define SCOPE_MIN_TIME_RANGE 10

#include <iostream>
#include <string>
#include <canvas/element.h>
#include <sstream>
#include <iomanip>

using namespace std;

class textElement;

class scopeElement : public element
{
public:
    scopeElement( int px, int py, int width, int height );
    void process();
    void draw();

    uint16_t *getDataPtr();
    uint32_t *getTimePtr();
    uint32_t getBufferSize();

    //int32_t getMicrosecondsDelay();
    uint32_t getAveragingRequest();

    void clearSweeps();

    void increaseTimeRange();
    void decreaseTimeRange();
    void increaseVolt();
    void decreaseVolt();
    void increaseVoltRange();
    void decreaseVoltRange();

    void setTriggerLevelV( float trigV );
    float getTriggerLevelV();
    void increaseTrigV( float amount );
    void decreaseTrigV( float amount );
    void increaseTrigOffset();
    void decreaseTrigOffset();
    void toggleTriggerMode();
    void triggerSmoothingDecrease();
    void triggerSmoothingIncrease();

private:
    int posx;   //top left corner x in pixels
    int posy;   //top left corner y in pixels
    int dx;   //width-1 in pixels
    int dy;   //height-1 in pixels

    uint16_t clampXmin;
    uint16_t clampXmax;
    uint16_t clampYmin;
    uint16_t clampYmax;

    uint16_t    displayWidth = 1024;
    uint16_t    displayHeight = 1024;
    uint32_t    dataMax = 4096;             //raw data input buffer length
    uint32_t    averagingRequest = 1;       //average 1 datapoint = no averaging

    int8_t      autoAdjustedDelayUpOrDown = 0;
    int8_t      autoAdjustedDelayPause = 0;

    //raw data and trime input buffers: per cycle *data and *time buffers are processed into
    uint16_t    *data; //[dataMax];     *** UNITS: daq
    uint32_t    *time; //[dataMax];     *** UNITS: us

    //time is defined in us --> no floats needed. maybe put this somewhere else...
    uint32_t    Tdisp = 10240;      //time per full displayed element --> 10240 us are displayed
    float       dTdisp;             //time per pixel is divided --> float
    float       minT;
    float       maxT;

    float       conversionScale;          //conversion factor from 12 bit ADC values to volts
    float       pixelsPerVolts;           //display scale: (vertical) pixels per voltage range

    float       lowV = 0.0;
    float       highV = 3.0;
    float       deltaV;

    float       yScale = 100.0;
    int         yOffset = 0;

    float       triggerLevelV;      //in V
    float       triggerLevel;       //bins
    float       triggerTime;        //time when trigger is triggered
    int32_t     triggerOffset = 0;     //to move trigger pos  (restrict to central part of data?)
    uint16_t    triggerSmoothing = 1;
    bool        triggerAscending = true;

    uint32_t    triggerI;           //horiz pixel trigger pos


    bool        somethingNewToDisplay = false;
    uint8_t     *pixelValid; //[displayWidth];
    uint16_t    *pixelValue; //[displayWidth];      in pixels
    float       *pixelTime;  //[displayWidth];

    bool        doAveraging = false;
    double      *pixelStatValues;
    uint32_t    *pixelStatAvgCount;

    string      debugString;

    textElement *timeBaseElem;
    textElement *voltBaseElem;
    textElement *samplingElem;
    textElement *triggerElem;

    textElement *maxTextElem;
    textElement *debugTextElem;
};

class progressBarElement : public element
{
public:
    progressBarElement( int px, int py, int dx, int dy );
    void draw();

    void setPercent( uint8_t percent );


private:

//static:
    int posx;   //top left corner x in pixels
    int posy;   //top left corner x in pixels
    int dx;   //width-1 in pixels
    int dy;   //height-1 in pixels

    int current_width;

    uint32_t    color = 0x00FF00;
};


class boxElement : public element
{
public:
    boxElement( int px, int py, int dx, int dy );
    void draw();

private:

//static:
    int posx;   //top left corner x in pixels
    int posy;   //top left corner x in pixels
    int dx;   //width-1 in pixels
    int dy;   //height-1 in pixels

    uint32_t    lineColor;
    uint32_t    fillColor;
    bool        hasFill = true;
};



class textElement : public element
{
public:
    textElement( int px, int py, int characterCount, uint8_t scale );
    void setText( string newText );
    void draw();

private:

    string      text;
    stringstream stream;

    uint32_t    *textBitmap;
    uint32_t    maxCharCount;   // 1 character is 5 wide
    uint8_t     textScale;
    uint32_t    textColor = 0xFFFFFF;

    int xpos;
    int ypos;
    int width;
    int height;

};




#endif
