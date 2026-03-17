// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */



#include <iostream>
#include <sstream>
#include <fstream>      //ifstream
#include <iomanip>      //setw
#include <filesystem>
#include <algorithm>    //remove_if
#include <bitset>

#include "utilities.h"


int decimalConverter( float *decimal )
{
  int counter = 0;
  float dec = *decimal;

  if (dec < 1 )
  {
    while( dec < 1 )
    {
      counter+=3;
      dec *= 1000;
    }
  }
  else
  {
    while( dec > 1 )
    {
      counter-=3;
      dec /= 1000;
    }
    counter +=3;
    dec *=1000;
  }

  *decimal = dec;
  return counter*-1;
}


vector<uint32_t> getUint32( vector<byte> bytes )
{
    uint32_t temp;
    vector<uint32_t> result;
    for (size_t i = 0; i < bytes.size(); i+=4 )
    {
        temp = ((uint32_t)bytes[i+0] << 24) | ((uint32_t)bytes[i+1] << 16) | ((uint32_t)bytes[i+2] << 8) | ((uint32_t)bytes[i+3]);
        result.push_back( temp );
    }
    return result;
}


string getHexString( vector<byte> bytes, uint8_t bytesGrouped )
{
    stringstream stream;
    uint8_t counter = 0;

    for( byte c : bytes )
    {
        uint32_t c2 = (uint32_t)c;
        if ( counter % bytesGrouped == 0 )
        {
            stream << "0x" << hex << setw(2) << setfill('0') << c2 << " " ;
        }
        else
        {
            stream << hex << setw(2) << setfill('0') << c2 << " " ; //setw(2) << setfill('0') <<
        }

        if ( (counter+1) % bytesGrouped == 0 )
        {
            stream << endl;
        }
        counter++;
    }
    return stream.str();
}

string getFileContentString( string path )
{
    //remove null characters
    path.erase( remove_if( path.begin(), path.end(), [](unsigned char c){ return !isprint(c); }), path.end() );
    ifstream file(path);

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();
    return buffer.str();
}


vector<byte> getFileContentBin( string path )
{
    //remove null characters

    path.erase( remove_if( path.begin(), path.end(), [](unsigned char c){ return !isprint(c); }), path.end() );
    filesystem::path filepath{path};
    auto length = std::filesystem::file_size(filepath);

    //cout << "length: " << length << endl;

    ifstream file( path, std::ios::binary );
    vector<byte> buffer;

    if( file )
    {
        byte x;
        int counter = 0;
        while( !file.fail() )
        {
            file.read( (char*)&x, 1);
            if( !file.fail() )
            {
                buffer.push_back( x );
                counter++;
            }
        }

        //cout << "file size: " << counter << endl;
        file.close();
    }
    else
    {
        cout << "file not open" << endl;
        file.close();
    }
    return buffer;
}


void printBin32( uint32_t data )
{
    uint8_t *bytes = (uint8_t*)&data;

    cout << getBinString( bytes[3] ) << " ";
    cout << getBinString( bytes[2] ) << " ";
    cout << getBinString( bytes[1] ) << " ";
    cout << getBinString( bytes[0] ) << " (dec: " << dec << data << ")";
}

string getBinString( uint8_t byte )
{
    stringstream stream;
    std::bitset<8> bits{byte};
    //cout << dec << " " << (uint32_t)byte << " ";
    stream << "0b";
    stream << bits[7];
    stream << bits[6];
    stream << bits[5];
    stream << bits[4];
    stream << " ";
    stream << bits[3];
    stream << bits[2];
    stream << bits[1];
    stream << bits[0];

    //cout << " 0x" << hex << setw(2) << setfill('0') << (uint32_t)byte;
    return stream.str();
}
