#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;
class Food;
class Compiler;

class Actor: public GraphObject {
public:
    
    Actor (int imageID, int startX, int startY, Direction dir, int depth, StudentWorld *studentWorld)
    : GraphObject(imageID, startX, startY, dir, depth), m_studentWorldPtr(studentWorld), m_doSomethingCalled(false) {}
    virtual ~Actor() {}
    
    // Action to perform each tick.
    virtual void doSomething() = 0;
    
    // Is this actor dead?
    virtual bool isDead() const {return false;}
    
    // Does this actor block movement?
    virtual bool blocksMovement() const {return false;}
    
    // Cause this actor to be be bitten, suffering an amount of damage.
    virtual void getBitten(int amt) {}
    
    // Cause this actor to be be poisoned.
    virtual void getPoisoned() {}
    
    // Cause this actor to be be stunned.
    virtual void getStunned() {}
    
    // Can this actor be picked up to be eaten?
    virtual bool isEdible() const {return false;}
    
    // Is this actor detected by an ant as a pheromone?
    virtual bool isPheromone(int colony) const {return false;}
    
    // Is this actor poison?
    virtual bool isPoison() const {return false;}
    
    // Is this actor water pool?
    virtual bool isWater() const {return false;}
    
    // Is this actor an enemy of an ant of the indicated colony?
    virtual bool isEnemy(int colony) const {return false;}
    
    // Is this actor detected as dangerous by an ant of the indicated colony?
    virtual bool isDangerous(int colony) const {return false;}
    
    // Is this actor the anthill of the indicated colony?
    virtual bool isAntHill(int colony) const {return false;}
    
    // getEnergy and updateEnergy declared in Actor instead of EnergyHolder so that Actor pointers can access them
    // Get this actor's amount of energy (for a Pheromone, same as strength).
    virtual int getEnergy() const {return 0;} //Cannot make pure virtual because then Pebble would have to declare this, so just return 0
    // Adjust this actor's amount of energy upward or downward.
    virtual void updateEnergy(int amt) {} //Cannot make pure virtual because then Pebble would have to declare this, so just make useless
    
    // Get this actor's world.
    StudentWorld* getWorld() const {return m_studentWorldPtr;}
    
    bool getDoSomethingCalled () {return m_doSomethingCalled;}
    void setDoSomethingCalled (bool doSomethingCalled) {m_doSomethingCalled = doSomethingCalled;}
    
    //These constants are passed into the StudentWorld's isActorAt function to clearly show what function StudentWorld will call
    const int isPoisonAt = 0;
    const int isWaterAt = 1;
    const int isPebbleAtLocation = 2;
    const int isEnemyAt = 3;
    const int isDangerAt = 4;
    const int isAntHillAt = 5;
private:
    bool m_doSomethingCalled;
    StudentWorld *m_studentWorldPtr;
};

class Pebble: public Actor {
public:
    
    Pebble (int imageID, int startX, int startY, StudentWorld *studentWorld)
    : Actor(imageID, startX, startY, right, 1, studentWorld) {}
    virtual ~Pebble() {}
    
    virtual void doSomething() {}
    virtual bool blocksMovement() const {return true;}
};

class EnergyHolder: public Actor {
public:
    
    EnergyHolder (int imageID, int startX, int startY, Direction dir, int depth, int energy, StudentWorld *studentWorld)
    : Actor(imageID, startX, startY, dir, depth, studentWorld), m_isDead(false), m_energy(energy) {}
    virtual ~EnergyHolder() {}
    
    virtual void doSomething() {}
    
    virtual bool isDead() const {return m_isDead;}
    void setDead() {m_isDead = true;}
    
    // Get this actor's amount of energy (for a Pheromone, same as strength).
    virtual int getEnergy() const {return m_energy;}
    
    // Adjust this actor's amount of energy upward or downward.
    virtual void updateEnergy(int amt);
    
    // Add an amount of food to this actor's location.
    void addFood(int amt);
    
    // Have this actor pick up an amount of food and eat it.
    bool pickupAndEatFood(int amt);
    
    // Does this actor become food when it dies?
    virtual bool becomesFoodUponDeath() const {return false;}
private:
    bool m_isDead;
    int m_energy;
};

class Food: public EnergyHolder {
public:
    
    Food (int imageID, int startX, int startY, int energy, StudentWorld *studentWorld)
    : EnergyHolder(imageID, startX, startY, right, 2, energy, studentWorld) {}
    virtual ~Food() {}
    
    virtual bool isEdible() const {return true;}
};

class AntHill: public EnergyHolder {
public:
    
    AntHill (int imageID, int startX, int startY, int colony, Compiler *program, StudentWorld *studentWorld)
    : EnergyHolder(imageID, startX, startY, right, 2, 8999, studentWorld), m_colony(colony), m_compiler(program) {}
    virtual ~AntHill() {}
    
    virtual void doSomething();
    virtual bool isAntHill(int colony) const {return (colony == m_colony);}
public:
    int m_colony;
    Compiler* m_compiler;
};

class Pheromone: public EnergyHolder {
public:
    
    Pheromone (int imageID, int startX, int startY, int colony, StudentWorld *studentWorld)
    : EnergyHolder(imageID, startX, startY, right, 2, 256, studentWorld), m_colony(colony) {}
    virtual ~Pheromone() {}
    
    virtual void doSomething() {updateEnergy(getEnergy()-1);}
    virtual bool isPheromone(int colony) const {return m_colony == colony;}
private:
    int m_colony;
};

class TriggerableActor: public EnergyHolder {
public:
    
    TriggerableActor (int imageID, int startX, int startY, StudentWorld *studentWorld)
    : EnergyHolder(imageID, startX, startY, right, 2, 0, studentWorld) {}
    virtual ~TriggerableActor() {}
    
    virtual bool isDangerous(int colony) const {return true;} //Regardless of colony, poison/water pools are considered dangerous
};

class WaterPool: public TriggerableActor {
public:
    
    WaterPool (int imageID, int startX, int startY, StudentWorld *studentWorld)
    : TriggerableActor(imageID, startX, startY, studentWorld) {}
    virtual ~WaterPool() {}
    
    virtual bool isWater() const {return true;}
};

class Poison: public TriggerableActor {
public:
    
    Poison (int imageID, int startX, int startY, StudentWorld *studentWorld)
    : TriggerableActor(imageID, startX, startY, studentWorld) {}
    virtual ~Poison() {}
    
    virtual bool isPoison() const {return true;}
};

class Insect: public EnergyHolder {
public:
    
    Insect (int imageID, int startX, int startY, int energy, StudentWorld *studentWorld)
    : EnergyHolder(imageID, startX, startY, Direction(randInt(1, 4)), 1, energy, studentWorld), m_stunnedTicks(0), m_distanceToWalk(0) {}
    virtual ~Insect() {}
    
    virtual void getBitten(int amt); //Subtract amt energy from actor
    virtual void getPoisoned(); //Make actor lose energy from poison
    virtual void getStunned() {m_stunnedTicks += 2;} //increase stunnedTicks
    
    // Get/Set the number of ticks this insect will sleep by the indicated amount.
    int getStunnedTicks() {return m_stunnedTicks;}
    void setStunnedTicks(int ticks) {m_stunnedTicks = ticks;}
    
    //Does this actor drop food when it dies?
    virtual bool becomesFoodUponDeath() const {return true;}
    
    // Set x,y to the coordinates of the spot one step in front of this insect.
    void getXYInFrontOfMe(int& x, int& y, Direction dir) const;
    
    // Move this insect one step forward if possible, and return true;
    // otherwise, return false without moving.
    virtual bool moveForwardIfPossible();
    
    //Get/set distance left to walk in direction actor is facing
    int getDistanceToWalk () {return m_distanceToWalk;}
    void setDistanceToWalk (int distance) {m_distanceToWalk = distance;}
private:
    int m_stunnedTicks;
    int m_distanceToWalk;
};

class Ant : public Insect
{
public:
    Ant (int imageID, int startX, int startY, int colony, Compiler* program, StudentWorld *studentWorld)
    : Insect(imageID, startX, startY, 1500, studentWorld), m_ic(0), m_lastRandomNumberGenerated(0), m_colony(colony), m_foodHolding(0), m_wasBlockedFromMoving(false), m_wasBitten(false), m_compiler(program) {}
    virtual ~Ant() {}
    
    virtual void doSomething();
    virtual bool isEnemy(int colony) const {return m_colony != colony;}
    virtual bool isDangerous(int colony) const {return m_colony != colony;}
    virtual bool moveForwardIfPossible(); //Try to move one step in direction it is facing
    virtual void getBitten(int amt);
private:
    int m_ic; //command counter
    int m_lastRandomNumberGenerated;
    int m_colony;
    int m_foodHolding;
    bool m_wasBlockedFromMoving;
    bool m_wasBitten;
    Compiler* m_compiler;
};

class Grasshopper: public Insect {
public:
    
    Grasshopper (int imageID, int startX, int startY, int energy, StudentWorld *studentWorld)
    : Insect(imageID, startX, startY, energy, studentWorld) {setDistanceToWalk(randInt(2, 10));}
    virtual ~Grasshopper() {}
    
    virtual void doSomething();
    virtual bool specificDoSomething() = 0; //Specialized auxiliary baby/adult grasshopper doSomething function
    virtual bool isEnemy(int colony) const {return true;} //Regardless of colony, grasshoppers are considered enemies
    virtual bool isDangerous(int colony) const {return true;} //Regardless of colony, grasshoppers are considered dangerous
};

class AdultGrasshopper: public Grasshopper {
public:
    
    AdultGrasshopper (int imageID, int startX, int startY, StudentWorld *studentWorld)
    : Grasshopper(imageID, startX, startY, 1600, studentWorld) {}
    virtual ~AdultGrasshopper() {}
    
    virtual void getPoisoned() {} //Adult grasshoppers don't get poisoned
    virtual void getStunned() {} //Adult grasshoppers don't get stunned
    virtual bool specificDoSomething(); //Make adult grasshopper bite or jump
    virtual void getBitten(int amt);
};

class BabyGrasshopper: public Grasshopper {
public:
    
    BabyGrasshopper (int imageID, int startX, int startY, StudentWorld *studentWorld)
    : Grasshopper(imageID, startX, startY, 500, studentWorld) {}
    virtual ~BabyGrasshopper() {}
    
    virtual bool specificDoSomething(); //Change into adult grasshopper if enough energy
    bool moveForwardIfPossible();
};

#endif // ACTOR_H_
