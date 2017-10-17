#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Field.h"
#include "Actor.h"
#include "Compiler.h"
#include <string>
#include <list>
#include <map>
using namespace std;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
	StudentWorld(string assetDir)
	 : GameWorld(assetDir), m_tickCount(0), m_winningAntNumber(0), antCompilers(0) {}
    
    ~StudentWorld() {}
    
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    //void addFood(int amt, int x, int y);
    
    // Add an actor to the world
    void addActor(Actor* a);
    
    // If an item that can be picked up to be eaten is at x,y, return a
    // pointer to it; otherwise, return a null pointer.  (Edible items are
    // only ever going be food.)
    Actor* getEdibleAt(int x, int y) const;
    
    // If a pheromone of the indicated colony is at x,y, return a pointer
    // to it; otherwise, return a null pointer.
    Actor* getPheromoneAt(int x, int y, int colony) const;
    
    //Call function based on whichFunction: 0 = isPoisonAt, 1 = isWaterAt, 2 = isPebbleAt, 3 = isEnemyAt, 4 = isDangerAt, 5 = isAnthillAt
    bool isActorAt(int x, int y, int colony, int whichFunction) const;
    
    //Is poison at x, y
    bool isPoisonAt(list<Actor*>::const_iterator tempIterator, int colony) const;
    
    //Is water pool at x, y
    bool isWaterAt(list<Actor*>::const_iterator tempIterator, int colony) const;
    
    // Is a pebble at x, y (can actor move there?)
    bool isPebbleAtLocation(list<Actor*>::const_iterator tempIterator, int colony) const;
    
    // Is an enemy of an ant of the indicated colony at x,y?
    bool isEnemyAt(list<Actor*>::const_iterator tempIterator, int colony) const;
    
    // Is something dangerous to an ant of the indicated colony at x,y?
    bool isDangerAt(list<Actor*>::const_iterator tempIterator, int colony) const;
    
    // Is the anthill of the indicated colony at x,y?
    bool isAntHillAt(list<Actor*>::const_iterator tempIterator, int colony) const;
    
    // Bite an enemy of an ant of the indicated colony at me's location
    // (other than me; insects don't bite themselves).  Return true if an
    // enemy was bitten.
    bool biteEnemyAt(Actor* me, int colony, int biteDamage);
    
    // Record another ant birth for the indicated colony.
    void increaseScore(int colony);
    
private:
    //Make sure doSomething() doesn't get called twice for all actors
    void resetActors();
    void callActorsActions();
    void removeDeadActors();
    
    //Choose which function isActorAt should call
    bool whichActorAtFunction(int whichFunction, list<Actor*>::const_iterator tempIterator, int colony) const;
    
    void setDisplayText();
    string formatDisplayText(int winningAntNumber);
    
    map<pair<int, int>, list<Actor*>> m_actorList;
    int m_tickCount;
    int m_winningAntNumber;
    int m_colonyScores[4];
    Compiler* m_comp[4];
    int antCompilers; //Number of ant programs given
};

#endif // STUDENTWORLD_H_
