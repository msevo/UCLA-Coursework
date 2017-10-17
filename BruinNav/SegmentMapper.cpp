#include "provided.h"
#include <vector>
#include "MyMap.h"
using namespace std;

class SegmentMapperImpl
{
public:
	SegmentMapperImpl();
	~SegmentMapperImpl();
	void init(const MapLoader& ml);
	vector<StreetSegment> getSegments(const GeoCoord& gc) const;
private:
    MyMap<GeoCoord, vector<StreetSegment>> segmentMap;
};

SegmentMapperImpl::SegmentMapperImpl()
{
}

SegmentMapperImpl::~SegmentMapperImpl()
{
}

void SegmentMapperImpl::init(const MapLoader& ml)
{
    for (int i = 0; i < ml.getNumSegments(); i++) {
        StreetSegment tempSegment;
        ml.getSegment(i, tempSegment);
        
        vector<StreetSegment> *tempStreetSegments1 = segmentMap.find(tempSegment.segment.start);
        if (tempStreetSegments1 == nullptr) {
            vector<StreetSegment> newVectorOfSegs;
            tempStreetSegments1 = &newVectorOfSegs;
        }
        tempStreetSegments1->push_back(tempSegment);
        segmentMap.associate(tempSegment.segment.start, *tempStreetSegments1);
        
        vector<StreetSegment> *tempStreetSegments2 = segmentMap.find(tempSegment.segment.end);
        if (tempStreetSegments2 == nullptr) {
            vector<StreetSegment> newVectorOfSegs;
            tempStreetSegments2 = &newVectorOfSegs;
        }
        tempStreetSegments2->push_back(tempSegment);
        segmentMap.associate(tempSegment.segment.end, *tempStreetSegments2);
        
        for (int j = 0; j < tempSegment.attractions.size(); j++) {
            vector<StreetSegment> *tempStreetSegments3 = segmentMap.find(tempSegment.attractions[j].geocoordinates);
            if (tempStreetSegments3 == nullptr) {
                vector<StreetSegment> newVectorOfSegs;
                tempStreetSegments3 = &newVectorOfSegs;
            }
            tempStreetSegments3->push_back(tempSegment);
            segmentMap.associate(tempSegment.attractions[j].geocoordinates, *tempStreetSegments3);
        }
    }
}

vector<StreetSegment> SegmentMapperImpl::getSegments(const GeoCoord& gc) const
{
    const vector<StreetSegment> *tempStreetSegmentsPtr = segmentMap.find(gc);
    if (tempStreetSegmentsPtr != nullptr) {
        return *tempStreetSegmentsPtr;
    } else {
        vector<StreetSegment> emptySegments;
        return emptySegments;
    }
}

//******************** SegmentMapper functions ********************************

// These functions simply delegate to SegmentMapperImpl's functions.
// You probably don't want to change any of this code.

SegmentMapper::SegmentMapper()
{
	m_impl = new SegmentMapperImpl;
}

SegmentMapper::~SegmentMapper()
{
	delete m_impl;
}

void SegmentMapper::init(const MapLoader& ml)
{
	m_impl->init(ml);
}

vector<StreetSegment> SegmentMapper::getSegments(const GeoCoord& gc) const
{
	return m_impl->getSegments(gc);
}
