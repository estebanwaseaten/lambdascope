// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */


#ifndef utilities_h
#define utilities_h

#include <vector>
#include <string>

using namespace std;

int decimalConverter( float *decimal );
string getFileContentString( string path );
vector<byte> getFileContentBinBytes( string path );
vector<uint32_t> getFileContentBinFourBytes( string path );


void printBin32( uint32_t data );
string getBinString( uint8_t byte );
















#endif
