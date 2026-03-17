// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *
 * Copyright (C) 2025 Daniel Wegkamp
 */


#ifndef debug_h
#define debug_h

#include <iostream>


//#undefine DEBUG
#define DEBUG
//#define QUIET

#ifdef DEBUG
  #ifdef QUIET
    #define DEBUG_MSG(str) do { std::ofstream debugfile;
                                debugfile.open("debug", std::ios_base::app);
                                debugfile << str << std::endl; } while( false )
  #else
    #define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
  #endif
#else
  #define DEBUG_MSG(str) do { } while ( false )
#endif

#endif
