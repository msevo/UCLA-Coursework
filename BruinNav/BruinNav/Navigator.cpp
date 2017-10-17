#include "provided.h"
#include <string>
#include <vector>
#include <queue>
#include "support.h"
#include "MyMap.h"
using namespace std;

class NavigatorImpl
{
public:
    NavigatorImpl();
    ~NavigatorImpl();
    bool loadMapData(string mapFile);
    NavResult navigate(string start, string end, vector<NavSegment>& directions) const;
private:
    double getFScore(const GeoCoord& coord, const GeoCoord& parentCoord, const GeoCoord& destinationCoord, double parentFScore) const;
    bool listContainsCoord(vector<GeoCoord> coordList, const GeoCoord& coord) const;
    void pathFromEndToStart (const GeoCoord& coord, const GeoCoord& startCoord, const GeoCoord& destinationCoord, MyMap<GeoCoord, GeoCoord>& locationOfPreviousWayPoint, vector<NavSegment> &directions) const;
    string getTurnDirection(double degrees) const;
    MapLoader mLoader;
    AttractionMapper attMapper;
    SegmentMapper segMapper;
};

NavigatorImpl::NavigatorImpl()
{
}

NavigatorImpl::~NavigatorImpl()
{
}

bool NavigatorImpl::loadMapData(string mapFile)
{
    if (!mLoader.load(mapFile)) {
        return false; //Error!
    }
    attMapper.init(mLoader);
    segMapper.init(mLoader);
    return true;
}

double NavigatorImpl::getFScore(const GeoCoord& coord, const GeoCoord& parentCoord, const GeoCoord& destinationCoord, double parentFScore) const {
    double parentGScore = parentFScore - distanceEarthMiles(parentCoord, destinationCoord); //parentF - parentH = parentGScore
    double gScore = distanceEarthMiles(coord, parentCoord) + parentGScore;
    double hScore = distanceEarthMiles(coord, destinationCoord); //Estimated distance to final destination
    return (gScore + hScore); //return FScore
}

bool NavigatorImpl::listContainsCoord(vector<GeoCoord> coordList, const GeoCoord& coord) const {
    for (int i = 0; i < coordList.size(); i++) {
        if (coordList[i] == coord) {
            return true; //coord is in coordList!
        }
    }
    return false;
}

string NavigatorImpl::getTurnDirection(double degrees) const {
    if (degrees < 180.0) {
        return "left";
    } else {
        return "right";
    }
}

void NavigatorImpl::pathFromEndToStart (const GeoCoord& key, const GeoCoord& startCoord, const GeoCoord& destinationCoord, MyMap<GeoCoord, GeoCoord>& locationOfPreviousWayPoint, vector<NavSegment> &directions) const {
    
    GeoCoord *currentCoord = locationOfPreviousWayPoint.find(key); //street segment will go from currentCoord to key
    if (currentCoord == nullptr) {
        return; //base case
    }
    
    pathFromEndToStart(*currentCoord, startCoord, destinationCoord, locationOfPreviousWayPoint, directions);
    
    vector<StreetSegment> streetSegs1 = segMapper.getSegments(key);
    vector<StreetSegment> streetSegs2 = segMapper.getSegments(*currentCoord);
    StreetSegment streetSeg;
    
    for (int i = 0; i < streetSegs1.size(); i++) {
        for (int j = 0; j < streetSegs2.size(); j++) {
            if (streetSegs1[i] == streetSegs2[j]) {
                //This found the street segment that both key and currentCoord are located on
                streetSeg = streetSegs1[i];
                streetSeg.segment.start = *currentCoord;
                streetSeg.segment.end = key;
            }
        }
    }
    
    if (directions.size() != 0 && directions.back().m_streetName != streetSeg.streetName) {
        //Changing streets, so add Turn-style NavSegment
        NavSegment newNavSeg(getTurnDirection(angleBetween2Lines(directions.back().m_geoSegment, streetSeg.segment)), streetSeg.streetName);
        directions.push_back(newNavSeg);
    }
    //Add Proceed-style NavSegment
    NavSegment newNavSeg(directionOfLine(streetSeg.segment), streetSeg.streetName, distanceEarthMiles(*currentCoord, key), streetSeg.segment);
    directions.push_back(newNavSeg);
}

NavResult NavigatorImpl::navigate(string start, string end, vector<NavSegment> &directions) const
{
    GeoCoord startCoord;
    if(!attMapper.getGeoCoord(start, startCoord)) {return NAV_BAD_SOURCE;}
    GeoCoord destinationCoord;
    if(!attMapper.getGeoCoord(end, destinationCoord)) {return NAV_BAD_DESTINATION;}
    
    MyMap<GeoCoord, GeoCoord> locationOfPreviousWayPoint;
    
    //I use A* as my route-finding algorithm. The following objects are needed for its implementation.
    //F score is basically determined by adding how long the path to a GeoCoord is from the start and approximately how long the path is from the GeoCoord to the end
    vector<GeoCoord> openCoordList;     // declare open list of GeoCoords
    vector<GeoCoord> closedCoordList;     // declare closed list of GeoCoords
    vector<GeoCoord> openParentCoordList;     // declare open list of parent GeoCoords
    vector<GeoCoord> closedParentCoordList;     // declare closed list of parent GeoCoords
    vector<double> openFScoreList;     // declare open list of GeoCoords' F scores (A*)
    vector<double> closedFScoreList;     // declare closed list of GeoCoords' F scores (A*)
    openCoordList.push_back(startCoord);
    openParentCoordList.push_back(startCoord); //First GeoCoord has itself as a parent
    openFScoreList.push_back(getFScore(startCoord, startCoord, destinationCoord, distanceEarthMiles(startCoord, destinationCoord)));
    do {
        GeoCoord currentCoord = openCoordList.front();
        GeoCoord currentParentCoord = openParentCoordList.front();
        double currentFScore = openFScoreList.front();
        int tempLowestFScorePos = 0;
        for (int i = 0; i < openFScoreList.size(); i++) {
            if (openFScoreList[i] <= openFScoreList[tempLowestFScorePos] || listContainsCoord(openCoordList, destinationCoord)) {
                tempLowestFScorePos = i; //GeoCoord in open list with lowest F score or the destination GeoCoord found
            }
        }
        
        //Set new currentCoord
        currentCoord = openCoordList[tempLowestFScorePos];
        currentParentCoord = openParentCoordList[tempLowestFScorePos];
        currentFScore = openFScoreList[tempLowestFScorePos];
        //Add it to closed list
        closedCoordList.push_back(currentCoord);
        closedParentCoordList.push_back(currentParentCoord);
        closedFScoreList.push_back(currentFScore);
        //Remove it from open list
        openCoordList.erase(openCoordList.begin()+tempLowestFScorePos);
        openParentCoordList.erase(openParentCoordList.begin()+tempLowestFScorePos);
        openFScoreList.erase(openFScoreList.begin()+tempLowestFScorePos);
        
        if (listContainsCoord(closedCoordList, destinationCoord)) {
            directions.clear();
            pathFromEndToStart(destinationCoord, startCoord, destinationCoord, locationOfPreviousWayPoint, directions);
            return NAV_SUCCESS; //Destination coord found
        }
        
        //Find all adjacent GeoCoords
        vector<GeoCoord> adjacentCoords;
        vector<StreetSegment> tempStreetSegments = segMapper.getSegments(currentCoord);
        for (int i = 0; i < tempStreetSegments.size(); i++) {
            if (!(currentCoord == tempStreetSegments[i].segment.start)) {
                adjacentCoords.push_back(tempStreetSegments[i].segment.start);
            }
            if (!(currentCoord == tempStreetSegments[i].segment.end)) {
                adjacentCoords.push_back(tempStreetSegments[i].segment.end);
            }
            for (int j = 0; j < tempStreetSegments[i].attractions.size(); j++) {
                if (!(currentCoord == tempStreetSegments[i].attractions[j].geocoordinates) && tempStreetSegments[i].attractions[j].geocoordinates == destinationCoord) {
                    adjacentCoords.push_back(tempStreetSegments[i].attractions[j].geocoordinates);
                }
            }
        }
        
        //Add unexplored adjacent GeoCoords to open list and map them to the currentCoord
        for (int i = 0; i < adjacentCoords.size(); i++) {
            if (!listContainsCoord(closedCoordList, adjacentCoords[i]) && !listContainsCoord(openCoordList, adjacentCoords[i])) {
                openCoordList.push_back(adjacentCoords[i]);
                openParentCoordList.push_back(currentCoord); //currentCoord is the new adjacent GeoCoord's parent
                openFScoreList.push_back(getFScore(adjacentCoords[i], currentCoord, destinationCoord, currentFScore));
                locationOfPreviousWayPoint.associate(adjacentCoords[i], currentCoord); //Associate new adjacent GeoCoord to parent GeoCoord
            }
        }
    } while (!openCoordList.empty());
    
	return NAV_NO_ROUTE;  //No route found from start to end
}

//******************** Navigator functions ************************************

// These functions simply delegate to NavigatorImpl's functions.
// You probably don't want to change any of this code.

Navigator::Navigator()
{
    m_impl = new NavigatorImpl;
}

Navigator::~Navigator()
{
    delete m_impl;
}

bool Navigator::loadMapData(string mapFile)
{
    return m_impl->loadMapData(mapFile);
}

NavResult Navigator::navigate(string start, string end, vector<NavSegment>& directions) const
{
    return m_impl->navigate(start, end, directions);
}
