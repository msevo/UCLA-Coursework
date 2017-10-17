//
//  support.hpp
//  Project 4
//
//  Created by Marko Sevo on 3/14/17.
//  Copyright © 2017 Marko Sevo. All rights reserved.
//

#ifndef support_h
#define support_h

#include <string>
#include <stdio.h>
#include <ctype.h>
#include "provided.h"
using namespace std;

string toLower(const string text);
string directionOfLine(const GeoSegment& gs);

bool operator< (const GeoCoord& coord1, const GeoCoord& coord2);
bool operator== (const GeoCoord& coord1, const GeoCoord& coord2);
bool operator== (const StreetSegment& seg1, const StreetSegment& seg2);

#endif /* support_h */
