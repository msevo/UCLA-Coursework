#include "provided.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

class MapLoaderImpl
{
public:
	MapLoaderImpl();
	~MapLoaderImpl();
	bool load(string mapFile);
	size_t getNumSegments() const;
	bool getSegment(size_t segNum, StreetSegment& seg) const;
private:
    vector<StreetSegment> streetSegments;
};

MapLoaderImpl::MapLoaderImpl()
{
}

MapLoaderImpl::~MapLoaderImpl()
{
}

bool MapLoaderImpl::load(string mapFile)
{
    ifstream file(mapFile);
    if (!file) {
        return false;
    }
    int lineCounter = 0;
    string lineString;
    int tempNumberOfAttractions = 0;
    while (getline(file, lineString)) {
        if (lineCounter == 0) {
            StreetSegment newStreetSegment;
            streetSegments.push_back(newStreetSegment);
            streetSegments.back().streetName = lineString;
            lineCounter++;
        } else if (lineCounter == 1) {
            int i; //End of first latitude
            for (i = 0; (lineString[i] != ',' && lineString[i] != ' '); i++) {}
            int j; //Beginning of first longitude
            for (j = i; (lineString[j] == ',' || lineString[j] == ' '); j++) {}
            int k; //End of first longitude
            for (k = j; (lineString[k] != ',' && lineString[k] != ' '); k++) {}
            GeoCoord tempCoord1(lineString.substr(0, 10), lineString.substr(j, (k-j)));
            streetSegments.back().segment.start = tempCoord1;
            int l; //Begninning of second latitude
            for (l = k; (lineString[l] == ',' || lineString[l] == ' '); l++) {}
            int m; //End of second latitude
            for (m = l; (lineString[m] != ',' && lineString[m] != ' '); m++) {}
            int n; //Beginning of second longitude
            for (n = m; (lineString[n] == ',' || lineString[n] == ' '); n++) {}
            GeoCoord tempCoord2(lineString.substr(l, (m-l)), lineString.substr(n));
            streetSegments.back().segment.end = tempCoord2;
            lineCounter++;
        } else if (lineCounter == 2) {
            if (stoi(lineString) == 0) {
                lineCounter = 0; //No attractions, so next line will be new street
            } else {
                lineCounter = 3; //Next line(s) will list attractions, so set lineCounter to 3 and process them in next if statement
                tempNumberOfAttractions = stoi(lineString);
            }
        } else if (lineCounter == 3) {
            tempNumberOfAttractions--;
            if (tempNumberOfAttractions <= 0) {
                lineCounter = 0;
            }
            Attraction tempAttraction;
            int i; //End of attraction name
            for (i = 0; lineString[i] != '|'; i++) {}
            tempAttraction.name = lineString.substr(0, i);
            int j; //End of latitude
            for (j = i+1; (lineString[j] != ',' && lineString[j] != ' '); j++) {}
            int k; //Beginning of longitude
            for (k = j; (lineString[k] == ',' || lineString[k] == ' '); k++) {}
            GeoCoord tempCoord1(lineString.substr(i+1, (j-i-1)), lineString.substr(k));
            tempAttraction.geocoordinates = tempCoord1;
            streetSegments.back().attractions.push_back(tempAttraction);
        }
    }
    return true;
}

size_t MapLoaderImpl::getNumSegments() const
{
    return streetSegments.size();
}

bool MapLoaderImpl::getSegment(size_t segNum, StreetSegment &seg) const
{
    if (segNum < streetSegments.size()) {
        seg = streetSegments[segNum];
        return true;
    } else {
        return false;
    }
}

//******************** MapLoader functions ************************************

// These functions simply delegate to MapLoaderImpl's functions.
// You probably don't want to change any of this code.

MapLoader::MapLoader()
{
	m_impl = new MapLoaderImpl;
}

MapLoader::~MapLoader()
{
	delete m_impl;
}

bool MapLoader::load(string mapFile)
{
	return m_impl->load(mapFile);
}

size_t MapLoader::getNumSegments() const
{
	return m_impl->getNumSegments();
}

bool MapLoader::getSegment(size_t segNum, StreetSegment& seg) const
{
   return m_impl->getSegment(segNum, seg);
}
