//
//  support.cpp
//  Project 4
//
//  Created by Marko Sevo on 3/14/17.
//  Copyright © 2017 Marko Sevo. All rights reserved.
//

#include "support.h"

string toLower(const string text) {
    string temp = "";
    for (int i = 0; i < text.size(); i++) {
        temp += tolower(text[i]);
    }
    return temp;
}

string directionOfLine(const GeoSegment& gs) {
    double degrees = angleOfLine(gs);
    if (degrees >= 0.0 && degrees <= 22.5) {
        return "east";
    } else if (degrees > 22.5 && degrees <= 67.5) {
        return "northeast";
    } else if (degrees > 67.5 && degrees <= 112.5) {
        return "north";
    } else if (degrees > 112.5 && degrees <= 157.5) {
        return "northwest";
    } else if (degrees > 157.5 && degrees <= 202.5) {
        return "west";
    } else if (degrees > 202.5 && degrees <= 247.5) {
        return "southwest";
    } else if (degrees > 247.5 && degrees <= 292.5) {
        return "south";
    } else if (degrees > 292.5 && degrees < 337.5) {
        return "southeast";
    } else {
        return "east";
    }
}

bool operator< (const GeoCoord& coord1, const GeoCoord& coord2) {
    
    if (coord1.latitude < coord2.latitude || (coord1.latitude == coord2.latitude && coord1.longitude < coord2.longitude)) {
        return true; //coord1 < coord 2
    } else {
        return false; //coord1 > coord 2
    }
}

bool operator== (const GeoCoord& coord1, const GeoCoord& coord2) {
    
    if (coord1.latitude == coord2.latitude && coord1.longitude == coord2.longitude) {
        return true; //coord1 < coord 2
    } else {
        return false; //coord1 > coord 2
    }
}

bool operator== (const StreetSegment& seg1, const StreetSegment& seg2) {
    
    if (seg1.segment.start == seg2.segment.start && seg1.segment.end == seg2.segment.end) {
        return true; //coord1 < coord 2
    } else {
        return false; //coord1 > coord 2
    }
}
