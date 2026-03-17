// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */


#include "custom_elements.h"


#include <canvas/canvas.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstring>

#include "text.h"


// SCOPE ELEMENT initialized with position and size.
scopeElement::scopeElement( int px, int py, int width, int height )
{
     posx = px;
     posy = py;
     dx = width - 1;
     dy = height - 1;

     displayWidth = width - 4;      //wegen rand oderso
     displayHeight = height - 4;

     clampXmin = 0 + 2;
     clampXmax = clampXmin + displayWidth;
     clampYmin = 0 + 2;
     clampYmax = clampYmin + displayHeight;

     dTdisp = (float)Tdisp / (float)displayWidth;  //needs to be recalc if Tdisp changes

     //display arrays:
     pixelValid = new uint8_t[displayWidth];
     pixelValue = new uint16_t[displayWidth];
     pixelTime = new float[displayWidth];            //is this pixel coord or actual value?? only needs to be calculated once?

     //statistics:
     doAveraging = true;
     pixelStatValues = new double[displayWidth];
     pixelStatAvgCount = new uint32_t[displayWidth];

     //raw data input buffers
     dataMax = 4096;
     data = new uint16_t[dataMax];
     time = new uint32_t[dataMax];

     // use  <algorithm> std::fill() or memset(spidata)?
     /*for (size_t i = 0; i < dataMax; i++)
     {
         data[i] = 0;
         time[i] = 0;
     }*/

     conversionScale = 2.511/3100.0;

     setTriggerLevelV( 1.0 );       //volts

    // triggerLevel = 0.5 / conversionScale;
     debugString = "hello";

     debugTextElem = new textElement( 10, 520, 50, 3 );
     this->addChild( debugTextElem );

     voltBaseElem = new textElement( px + width + 10, py, 23, 2 );
     timeBaseElem = new textElement( px + width + 10, py+40, 23, 2 );
     samplingElem = new textElement( px + width + 10, py+80, 23, 2 );
     maxTextElem = new textElement( px + width + 10, py+120, 23, 2 );

     this->addChild( maxTextElem );     //--> canvas will be set later if element is in the element tree
     this->addChild( timeBaseElem );
     this->addChild( voltBaseElem );
     this->addChild( samplingElem );

     triggerElem = new textElement( px + width + 10, py+160, 40, 1 );
     this->addChild( triggerElem );
}

void scopeElement::setTriggerLevelV( float trigV )
{
    triggerLevelV = trigV;
    triggerLevel = triggerLevelV/conversionScale;
    clearSweeps();
}

float scopeElement::getTriggerLevelV()
{
    return triggerLevelV;
}

void scopeElement::increaseTrigV( float amount )
{
    triggerLevelV += amount;
    triggerLevel = triggerLevelV/conversionScale;
    clearSweeps();
}

void scopeElement::decreaseTrigV( float amount )
{
    triggerLevelV -= amount;
    triggerLevel = triggerLevelV/conversionScale;
    clearSweeps();
}

void scopeElement::increaseTrigOffset()
{
    triggerOffset += Tdisp/20;
    triggerOffset = clamp( (int32_t)triggerOffset, -(int32_t)Tdisp/2, (int32_t)Tdisp/2);
    clearSweeps();
}

void scopeElement::decreaseTrigOffset()
{
    triggerOffset -= Tdisp/20;
    triggerOffset = clamp( (int32_t)triggerOffset, -(int32_t)Tdisp/2, (int32_t)Tdisp/2);
    clearSweeps();
 }

uint16_t *scopeElement::getDataPtr()
{
    return data;
}

uint32_t *scopeElement::getTimePtr()
{
    return time;
}

void scopeElement::toggleTriggerMode()
{
    triggerAscending = !triggerAscending;
    clearSweeps();
}

void scopeElement::increaseTimeRange()
{
    //cout << "Tdisp: " << Tdisp << endl;
    if (Tdisp <= SCOPE_MAX_TIME_RANGE)
    {
        Tdisp *= 2;
        dTdisp = (float)Tdisp / (float)displayWidth;
    }
    triggerOffset = clamp( (int32_t)triggerOffset, -(int32_t)Tdisp/2, (int32_t)Tdisp/2);
    autoAdjustedDelayUpOrDown = 0;
    autoAdjustedDelayPause = 0;
    clearSweeps();

    cout << "Tdisp: " << Tdisp << " us" << endl;
}
void scopeElement::decreaseTimeRange()
{
    //cout << "Tdisp: " << Tdisp << endl;
    if( Tdisp > SCOPE_MIN_TIME_RANGE )
    {
        Tdisp /= 2;
        dTdisp = (float)Tdisp / (float)displayWidth;
    }
    triggerOffset = clamp( (int32_t)triggerOffset, -(int32_t)Tdisp/2, (int32_t)Tdisp/2);
    autoAdjustedDelayUpOrDown = 0;
    autoAdjustedDelayPause = 0;
    clearSweeps();

    cout << "Tdisp: " << Tdisp << " us" << endl;
}

void scopeElement::increaseVolt()
{
    float delta = deltaV/30;
    highV += delta;
    lowV += delta;
    clearSweeps();

    cout << "Voltage range: " << lowV << " - " << highV << endl;
}
void scopeElement::decreaseVolt()
{
    float delta = deltaV/30;
    highV -= delta;
    lowV -= delta;
    clearSweeps();

    cout << "Voltage range: " << lowV << " - " << highV << endl;
}

void scopeElement::increaseVoltRange()
{
    //recalculate lowV, highV and deltaV
    deltaV = highV - lowV;

    if( 2*deltaV < SCOPE_MAX_VOLT_RANGE )
    {
        float centerV = lowV + 0.5 * deltaV;

        deltaV *= 2;
        lowV = centerV - 0.5 * deltaV;
        highV = centerV + 0.5 * deltaV;
    }



    clearSweeps();

    cout << "Voltage range: " << lowV << " - " << highV << endl;
}
void scopeElement::decreaseVoltRange()
{
    //recalculate lowV, highV and deltaV
    deltaV = highV - lowV;

    if( 0.5*deltaV > SCOPE_MIN_VOLT_RANGE )
    {
        float centerV = lowV + 0.5 * deltaV;

        deltaV *= 0.5;
        lowV = centerV - 0.5 * deltaV;
        highV = centerV + 0.5 * deltaV;
    }

    clearSweeps();
    cout << "Voltage range: " << lowV << " - " << highV << endl;
}

uint32_t scopeElement::getBufferSize()
{
    return dataMax;
}

uint32_t scopeElement::getAveragingRequest()
{
    return averagingRequest;
}

void scopeElement::clearSweeps()
{
    for (size_t i = 0; i < displayWidth; i++)
    {
        pixelStatValues[i] = 0;
        pixelStatAvgCount[i] = 0;
    }
}

void scopeElement::triggerSmoothingDecrease()
{
    triggerSmoothing = clamp( triggerSmoothing-1, 1, 128 );
    cout << "Decreased trigger smoothing to " << triggerSmoothing << endl;
}

void scopeElement::triggerSmoothingIncrease()
{
    triggerSmoothing = clamp( triggerSmoothing+1, 1, 128 );
    cout << "Increased trigger smoothing to " << triggerSmoothing << endl;
}

//time is in integer milliseconds
void scopeElement::process()
{
    //1. make sure we have to complete duration data buffer filled: if we measure "too fast" add some delay         ///delay could be averaging instead of ...
    // time[dataMax-1]-time[0] should be at least 3*Tdisp
    uint32_t delta = time[dataMax-1]-time[0];
    if( (delta < 3 * Tdisp) && autoAdjustedDelayPause == 0 )
    {
        if (autoAdjustedDelayUpOrDown < 0)
            autoAdjustedDelayPause = 1;

        autoAdjustedDelayUpOrDown = 1;

        averagingRequest *=2;

        averagingRequest = clamp((int)averagingRequest, 1, 65535);
        return;
    }
    if( (delta > 6 * Tdisp) && (averagingRequest != 1) && autoAdjustedDelayPause == 0 )
    {
        if (autoAdjustedDelayUpOrDown > 0)
            autoAdjustedDelayPause = 1;

        autoAdjustedDelayUpOrDown = -1;

        averagingRequest /=2;

        averagingRequest = clamp((int)averagingRequest, 1, 65535);
        return;
    }

    //2. remove time offset such that traces always start at zero
    uint32_t timeCorrection = time[0];
    for (size_t i = 0; i < dataMax; i++)
    {
        time[i] -= timeCorrection;
    }

    //3. find trigger level:
    //search from the middle of the trace outwards. simple search is difficult --> add some "trigger-smoothing"
    uint32_t dataMaxHalf = dataMax/2;
    int lower = dataMax/2;
    int upper = dataMax/2-1;
    int max = dataMax;

    double   dataOneLo, dataTwoLo, dataOneUp, dataTwoUp;
    double   timeOneLo, timeTwoLo, timeOneUp, timeTwoUp;
    uint32_t indexPlus;
    uint32_t indexMinus;

    triggerTime = round((time[dataMaxHalf] + time[dataMaxHalf+1])/2.0);         //default trigger in case the trigger-finder does not find it

    for( int i = 0; i < dataMaxHalf; i++ )
    {
        dataOneLo = 0.0;
        dataTwoLo = 0.0;
        timeOneLo = 0.0;
        timeTwoLo = 0.0;

        dataOneUp = 0.0;
        dataTwoUp = 0.0;
        timeOneUp = 0.0;
        timeTwoUp = 0.0;

        if ( triggerAscending )
        {
            for (int j = 0; j < triggerSmoothing; j++)                          //simple summation should suffice (averaging without dividing because divisor is constant)
            {
                dataOneLo += data[clamp( (lower   +j), 0, max )];   //larger value
                dataTwoLo += data[clamp( (lower-1 -j), 0, max )];   //smaller value
                timeOneLo += time[clamp( (lower   +j), 0, max )];
                timeTwoLo += time[clamp( (lower-1 -j), 0, max )];

                dataOneUp += data[clamp( upper   -j, 0, max )];
                dataTwoUp += data[clamp( upper+1 +j, 0, max )];
                timeOneUp += time[clamp( upper   -j, 0, max )];
                timeTwoUp += time[clamp( upper+1 +j, 0, max )];
            }
        }
        else
        {
            for (int j = 0; j < triggerSmoothing; j++)                          //simple summation should suffice (averaging without dividing because divisor is constant)
            {
                dataOneLo += data[clamp( (lower-1 -j), 0, max )];
                dataTwoLo += data[clamp( (lower   +j), 0, max )];
                timeOneLo += time[clamp( (lower-1 -j), 0, max )];
                timeTwoLo += time[clamp( lower   +j, 0, max )];

                dataOneUp += data[clamp( upper+1 +j, 0, max )];
                dataTwoUp += data[clamp( upper   -j, 0, max )];
                timeOneUp += time[clamp( upper+1 +j, 0, max )];
                timeTwoUp += time[clamp( upper   -j, 0, max )];
            }
        }

        //cout << dataOneLo << "-" << dataTwoLo << " --- " << dataOneUp << "-" << dataTwoUp << endl;

        dataOneLo/=triggerSmoothing;
        dataTwoLo/=triggerSmoothing;
        timeOneLo/=triggerSmoothing;
        timeTwoLo/=triggerSmoothing;
        dataOneUp/=triggerSmoothing;
        dataTwoUp/=triggerSmoothing;
        timeOneUp/=triggerSmoothing;
        timeTwoUp/=triggerSmoothing;


        if( (dataOneLo >= triggerLevel ) && (dataTwoLo < triggerLevel ) ) //have to make sure
        {
            //found trigger interval. Linear interpolation:
            float dT = timeOneLo - timeTwoLo;
            float dV = dataOneLo - dataTwoLo;
            float dV1 = triggerLevel - dataTwoLo;
            triggerTime = timeTwoLo + dV1*dT/dV;          //is in us
            triggerI = lower;
            i = dataMaxHalf;
        }
        else if( (dataOneUp <= triggerLevel ) && (dataTwoUp > triggerLevel ) )
        {
            //found trigger interval. Linear interpolation:
            float dT = timeTwoUp - timeOneUp;
            float dV = dataTwoUp - dataOneUp;
            float dV1 = triggerLevel - dataOneUp;
            triggerTime = timeOneUp + dV1*dT/dV;          //is in us
            triggerI = upper;
            i = dataMaxHalf;
        }
        upper++;
        lower--;
    }

    //cout << dataOneLo << "-" << dataTwoLo << " --- " << dataOneUp << "-" << dataTwoUp << "level: " << triggerLevel << " trigerI: " << triggerI << endl;


    // define center of screen time
    uint32_t    centerTime = triggerTime - triggerOffset;

    // display time axis:
    uint32_t    startTime = centerTime - Tdisp / 2.0;

    minT = startTime;
    maxT = startTime + Tdisp;

    float    intervalStartT, intervalStopT;     //averaging interval
    float    dTdispHalf = dTdisp / 2.0;        // this becomes zero when dTdisp is smaller than one!

    int         dataIndex = 0;
    double      dataAvg;
    int         dataAvgCount;
    float       finalScale = yScale * conversionScale;
    double      currentTime = startTime;

    // y Scaling:
    deltaV = highV - lowV;        //voltage range should be called Vdisp
    pixelsPerVolts = (float)displayHeight / deltaV;

    int     dataPointCount = 0;

    //cout << "displayWidth: " << displayWidth << endl;
    //cout << "dTdisp: " << dTdisp << endl;
    //cout << "dTdispHalf: " << dTdispHalf << endl;
    int debug_invalid = 0;

    for ( int i = 0; i < displayWidth; i++)     //loop through all pixels
    {
        pixelTime[i] = currentTime;
        currentTime += dTdisp;                      //in case of a 10000us display interval: 16us off --> 0,16% error is ok

        //define the averaging interval:
        intervalStartT = pixelTime[i] - dTdispHalf;
        intervalStopT = pixelTime[i] + dTdispHalf;

        //average and store value
        dataAvg = 0;
        dataAvgCount = 0;

        while( (time[dataIndex] <= intervalStopT) && (dataIndex < dataMax) )
        {
            if( time[dataIndex] > intervalStartT )   //average!!
            {
                dataAvg += data[dataIndex];
                dataAvgCount++;
            }
            else
            {
                //??
            }
            dataIndex++;
        }
        if( dataAvgCount > 0 )
        {
            double myMeasuredValue = pixelsPerVolts * ( highV - (conversionScale * dataAvg / (double)dataAvgCount) );
            pixelValue[i] = round( myMeasuredValue );     //conversionScale = 2.511/3100.0;
            pixelValue[i] = clamp( pixelValue[i], clampYmin, clampYmax );   //so it doesnt leak out of the window
            pixelValid[i] = 1;

            if( doAveraging )
            {
                //double      *pixelStatValues;
                //uint32_t    *pixelStatAvgCount;
                //uint16_t    *pixelValue;
                pixelStatValues[i] = pixelStatValues[i]*pixelStatAvgCount[i]/(pixelStatAvgCount[i]+1) + clamp(myMeasuredValue, (double)clampYmin, (double)clampYmax)/(pixelStatAvgCount[i]+1);
                pixelStatAvgCount[i]++;
            }

            dataPointCount++;
        //    cout << pixelValue[i] << "(" << i << ")";
        }
        else
        {
            pixelValid[i] = 0;
            debug_invalid++;
        }
    }

    //debugString = to_string(pixelValue[0]);// + " vs " + to_string(dataPointCount);

    //too many pixels are invalid....
    //cout << "invalid pixels: " << debug_invalid << endl;
    stringstream tempStream;
    tempStream.precision(2);
    tempStream << "Voltbase: " << lowV << " to " << highV << " V";
    voltBaseElem->setText( tempStream.str() );

    tempStream.clear();//clear any bits set
    tempStream.str(std::string());

    tempStream << "Timebase: " << Tdisp << " us";
    timeBaseElem->setText( tempStream.str() );

    tempStream.clear();//clear any bits set
    tempStream.str(std::string());

    tempStream << "Trigger: " << triggerLevelV << " V (" << triggerI << ")";
    triggerElem->setText( tempStream.str() );

    tempStream.clear();//clear any bits set
    tempStream.str(std::string());

    tempStream << time[0] << "-" << time[dataMax-1] << " / " << dataMax;
    debugTextElem->setText( tempStream.str() );

    tempStream.clear();//clear any bits set
    tempStream.str(std::string());

    float sampling = 1000.0*dataMax/(time[dataMax-1] - time[0]);    //in ksps

    tempStream << "rate: " << dec << fixed << sampling << " ksps";
    samplingElem->setText( tempStream.str() );
}


void scopeElement::draw()
{
    myCanvas->drawFrame( posx, posy, dx, dy );      //simply drawas a box

    if( doAveraging )
    {
        myCanvas->setLineColor( 0, 255, 0 );
        for ( int i = 0; i < displayWidth; i++)
        {
            if( pixelStatAvgCount[i] > 0 )
            {
                //myCanvas->drawPoint( i + posx, pixelStatValues[i] + posy );
                myCanvas->drawBox( i + posx - 1, pixelStatValues[i] - 1 + posy, 3, 3 );
            }
        }
    }


    uint16_t lastValidPixelY = 0;
    uint16_t lastValidPixelX = 0;
    uint8_t lastValidPixelExists = 0;

    uint16_t thisValidPixelY = 0;
    uint16_t thisValidPixelX = 0;

    myCanvas->setLineColor( 255, 255, 255 );
    myCanvas->setFillColor( 0, 0, 0 );

    //draw data curve (from one to next valid pixel)
    for ( int i = 0; i < displayWidth; i++)
    {

        if ( pixelValid[i] == 1 )
        {
            thisValidPixelY = pixelValue[i] + posy;
            thisValidPixelX = i + posx;                 //loop i is the cuurent x position within the element

            if ( lastValidPixelExists == 1)
            {
                //draw line from last pixel to this pixel: (thisValidPixelX, lastValidPixel) to (thisValidPixelX, thisValidPixel)
                //cout << "(" << i + posx << "," << pixelValue[i] << ") ";

                myCanvas->drawLine(  thisValidPixelX, thisValidPixelY, lastValidPixelX, lastValidPixelY );

            }

            //store pixelValue[i] into lastValidPixel
            lastValidPixelY = thisValidPixelY;
            lastValidPixelX = thisValidPixelX;
            lastValidPixelExists = 1;
        }
    }

    //draw datapoints:
    myCanvas->setLineColor( 255, 0, 0 );
    myCanvas->setFillColor( 255, 0, 0 );
    for ( int i = 0; i < displayWidth; i++)
    {
        if ( pixelValid[i] == 1 )
        {
            //cout << "(" << i + posx << "," << pixelValue[i] << ") ";
            //myCanvas->drawPoint( i + posx, pixelValue[i]-1 );
            myCanvas->drawBox( i + posx - 1, pixelValue[i]-1 + posy, 3, 3 );
        }
    }

    //display trigger level: posx, posy, dx, dy
    myCanvas->setLineColor( 255, 255, 255 );
    uint32_t trigY = round( pixelsPerVolts * (highV - triggerLevelV) ) + posy;
    myCanvas->drawStripedLine( posx, trigY , posx + dx, trigY, 5, 5 );
    uint16_t trigX = round( triggerOffset/dTdisp + dx/2);
    myCanvas->drawStripedLine( trigX + posx, posy , trigX + posx, posy + dy, 5, 5 );

    //myCanvas->drawTestbild();

}

// PROGRESS BAR ELEMENT
progressBarElement::progressBarElement( int px, int py, int width, int height )
{
    posx = px;
    posy = py;
    dx = width - 1;
    dy = height - 1;
}

void progressBarElement::setPercent( uint8_t percent )
{
    current_width = (float)percent / 100.0 * dx;
}

void progressBarElement::draw()
{
    myCanvas->setFillColor( color );
    myCanvas->drawBox( posx, posy, current_width, dy );        //optimized drawBox Routine IS OPAQUE!
}


// BOX ELEMENT
boxElement::boxElement( int px, int py, int width, int height )
{
    posx = px;
    posy = py;
    dx = width - 1;
    dy = height - 1;
    lineColor = 0xFFFFFF;
    fillColor = 0x808080;
}

void boxElement::draw()
{
    //std::cout << "boxElement::draw()" << std::endl;

    /*runs at 25 kHz */
    myCanvas->setLineColor( lineColor );
    myCanvas->setFillColor( fillColor );
    myCanvas->drawBox( posx, posy, dx, dy );        //optimized drawBox Routine (is opaque)!
    myCanvas->drawFrame( posx, posy, dx, dy );

    /*runs at 21 kHz *//*
    myCanvas->setLineColor( fillColor );
    for (size_t i = posy+1; i < posy+dimy-1; i++)
    {
        myCanvas->drawLine( posx, i, posx + dimx, i );
    }

    myCanvas->setLineColor( lineColor );
    //horiz
    myCanvas->drawLine( posx, posy, posx + dimx, posy );
    myCanvas->drawLine( posx, posy+dimy, posx + dimx, posy+dimy );
    //vert
    myCanvas->drawLine( posx, posy, posx, posy+dimy );
    myCanvas->drawLine( posx+dimx, posy, posx+dimx, posy+dimy );
    /**/
}


// TEXT ELEMENT
textElement::textElement(int px, int py, int characterCount, uint8_t scale )
{
    xpos = px;
    ypos = py;
    maxCharCount = characterCount;
    textScale = scale;

    width = textScale * (charx+2) * maxCharCount;
    height = textScale * (chary+2);

    textBitmap = new uint32_t[width*height];
}

void textElement::draw()
{
    myCanvas->setLineColor( 0xFFFFFF );
    myCanvas->setFillColor( 0xFFFFFF );
    myCanvas->drawFrame( xpos, ypos, width, height );

    //better:?
    //myCanvas->insertRegion( uint32_t *region, uint16_t x, uint16_t y, uint16_t dx, uint16_t dy);
    //myCanvas->insertRegion( myBitmap, xpos, ypos, width, height);
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            if (textBitmap[y*width + x] > 1)
            {
                myCanvas->drawPoint(xpos+x, ypos+y);
            }
        }
    }
}



void textElement::setText( string newText )
{
    text = newText;

    //1. clear old bitmap
    memset(textBitmap, 0, 4*width*height);

    //2. fill bitmap
    uint32_t minl = min((uint32_t)text.length(), maxCharCount);
    for( size_t i = 0; i < minl; ++i )     //loop through each word
    {
        int characterOffsetX = (charx+1) * i * textScale;

        if ( characterMap32.find(text[i]) != characterMap32.end() )
        {
            //cout << "found: ";
            for(size_t y = 0; y < chary; y++)
            {
                for(size_t x = 0; x < charx; x++)
                {
                    if ( characterMap32[text[i]][x+charx*y] == 1 )
                    {
                        //draw square of size scale
                        for (size_t sy = 0; sy < textScale; sy++)
                        {
                            for (size_t sx = 0; sx < textScale; sx++)
                            {
                                textBitmap[ textScale * width * (y+1) + sy*width + textScale * (x+1) + sx + characterOffsetX] = textColor;
                            }
                        }
                    }
                }
            }
        }
    }
}
