#include "Actor.h"
#include "StudentWorld.h"
#include "Compiler.h"
#include <cmath>

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

void EnergyHolder::updateEnergy(int amt) {
    m_energy = amt;
    if (m_energy <= 0) { //Make dead
        setDead();
        if (becomesFoodUponDeath()) {
            addFood(100);
            return;
        }
    }
}

void EnergyHolder::addFood(int amt) {
    Actor *tempFoodPtr = getWorld()->getEdibleAt(getX(), getY());
    if (tempFoodPtr != nullptr) {
        //Food object at location, so add to it
        tempFoodPtr->updateEnergy(tempFoodPtr->getEnergy()+100);
    } else {
        //No food at location, so make food object
        Actor *tempFoodPtr = new Food(IID_FOOD, getX(), getY(), 100, getWorld());
        getWorld()->addActor(tempFoodPtr);
    }
}

bool EnergyHolder::pickupAndEatFood(int amt) {
    Actor *tempFoodPtr = getWorld()->getEdibleAt(getX(), getY());
    if (tempFoodPtr != nullptr) {
        //Eat food from this existing food object
        if (tempFoodPtr->getEnergy() <= amt) {
            updateEnergy(getEnergy() + tempFoodPtr->getEnergy());
        } else {
            updateEnergy(getEnergy() + amt);
        }
        tempFoodPtr->updateEnergy(tempFoodPtr->getEnergy()-amt);
        return true;
    }
    return false;
}

void AntHill::doSomething() {
    setDoSomethingCalled(true);
    updateEnergy(getEnergy()-1);
    if (isDead()) {return;}
    pickupAndEatFood(10000);
    if (getEnergy() >= 2000) {
        //create ant
        updateEnergy(getEnergy()-1500);
        Actor *tempAntPtr;
        switch (m_colony) {
            case 0:
                tempAntPtr = new Ant(IID_ANT_TYPE0, getX(), getY(), m_colony, m_compiler, getWorld());
                break;
            case 1:
                tempAntPtr = new Ant(IID_ANT_TYPE1, getX(), getY(), m_colony, m_compiler, getWorld());
                break;
            case 2:
                tempAntPtr = new Ant(IID_ANT_TYPE2, getX(), getY(), m_colony, m_compiler, getWorld());
                break;
            case 3:
                tempAntPtr = new Ant(IID_ANT_TYPE3, getX(), getY(), m_colony, m_compiler, getWorld());
                break;
            default:
                break;
        }
        getWorld()->addActor(tempAntPtr);
        getWorld()->increaseScore(m_colony);
    }
}

void Insect::getBitten(int amt) {
    updateEnergy(getEnergy()-amt);
}

void Insect::getPoisoned() {
    updateEnergy(getEnergy()-150);
}

void Insect::getXYInFrontOfMe(int& x, int& y, Direction dir) const {
    switch (dir) {
        case up:
            y += 1;
            break;
        case down:
            y -= 1;
            break;
        case left:
            x -= 1;
            break;
        case right:
            x += 1;
            break;
        default:
            break;
    }
}

bool Insect::moveForwardIfPossible() {
    switch (getDirection()) {
        case up:
            if (!getWorld()->isActorAt(getX(), getY()+1, 0, isPebbleAtLocation)) { //can move in that direction?
                moveTo(getX(), getY()+1);
                setDistanceToWalk(getDistanceToWalk()-1);
            } else {
                setDistanceToWalk(0);
                return false;
            }
            break;
        case down:
            if (!getWorld()->isActorAt(getX(), getY()-1, 0, isPebbleAtLocation)) { //can move in that direction?
                moveTo(getX(), getY()-1);
                setDistanceToWalk(getDistanceToWalk()-1);
            } else {
                setDistanceToWalk(0);
                return false;
            }
            break;
        case left:
            if (!getWorld()->isActorAt(getX()-1, getY(), 0, isPebbleAtLocation)) { //can move in that direction?
                moveTo(getX()-1, getY());
                setDistanceToWalk(getDistanceToWalk()-1);
            } else {
                setDistanceToWalk(0);
                return false;
            }
            break;
        case right:
            if (!getWorld()->isActorAt(getX()+1, getY(), 0, isPebbleAtLocation)) { //can move in that direction?
                moveTo(getX()+1, getY());
                setDistanceToWalk(getDistanceToWalk()-1);
            } else {
                setDistanceToWalk(0);
                return false;
            }
            break;
        default:
            break;
    }
    return true;
}

void Ant::doSomething() {
    setDoSomethingCalled(true);
    updateEnergy(getEnergy()-1);
    if (getStunnedTicks() > 0) {
        setStunnedTicks(getStunnedTicks()-1);
        return;
    }
    int commandsExecuted = 0; //Maximum of 10 commands can be executed per doSomething() call
    while (commandsExecuted < 10) {
    Compiler::Command cmd;
    if (!m_compiler->getCommand(m_ic, cmd)) {
        setDead();
        return; // error - no such instruction!
    }
    switch (cmd.opcode) {
        case Compiler::moveForward:
            moveForwardIfPossible();
            ++m_ic;
            return;
            break;
        case Compiler::eatFood:
            if (m_foodHolding < 100) {
                //Less than 100 units of food so eat all of it
                updateEnergy(getEnergy()+m_foodHolding);
                m_foodHolding = 0;
            } else {
                updateEnergy(getEnergy()+100);
                m_foodHolding -= 100;
            }
            ++m_ic;
            return;
            break;
        case Compiler::dropFood: {
            Actor *tempFoodPtr = getWorld()->getEdibleAt(getX(), getY());
            if (tempFoodPtr != nullptr) { //Food object already at location so add to it
                tempFoodPtr->updateEnergy(tempFoodPtr->getEnergy()+m_foodHolding);
            } else { //No food object there so make new one
                tempFoodPtr = new Food(IID_FOOD, getX(), getY(), m_foodHolding, getWorld());
                getWorld()->addActor(tempFoodPtr);
            }
            m_foodHolding = 0;
            ++m_ic;
            return;
            break;
        }
        case Compiler::bite:
            getWorld()->biteEnemyAt(this, m_colony, 15);
            ++m_ic;
            return;
            break;
        case Compiler::pickupFood: {
            Actor *tempFoodPtr = getWorld()->getEdibleAt(getX(), getY());
            if (tempFoodPtr != nullptr) {
                //Eat food from this food object
                int amtPossibleToPickUp = 0;
                if (tempFoodPtr->getEnergy() < 400) {
                    amtPossibleToPickUp = tempFoodPtr->getEnergy();
                } else {
                    amtPossibleToPickUp = 400;
                }
                //Check if amtPossible to pick up would make m_foodHolding exceed 1800
                if ((m_foodHolding+amtPossibleToPickUp) <= 1800) {
                    m_foodHolding += amtPossibleToPickUp;
                    tempFoodPtr->updateEnergy(tempFoodPtr->getEnergy()-amtPossibleToPickUp);
                } else {
                    //Only eat as much of the food that would make m_holding 1800
                    tempFoodPtr->updateEnergy(tempFoodPtr->getEnergy()-(1800-m_foodHolding));
                    m_foodHolding = 1800;
                }
            }
            ++m_ic;
            return;
            break;
        }
        case Compiler::emitPheromone: {
            Actor *tempPheromonePtr = getWorld()->getPheromoneAt(getX(), getY(), m_colony);
            if (tempPheromonePtr != nullptr) { //Find pheromone of ant's colony
                tempPheromonePtr->updateEnergy(tempPheromonePtr->getEnergy()+256);
                if (tempPheromonePtr->getEnergy() > 768) {
                    tempPheromonePtr->updateEnergy(768);
                }
            } else {
                switch (m_colony) {
                    case 0:
                        tempPheromonePtr = new Pheromone(IID_PHEROMONE_TYPE0, getX(), getY(), m_colony, getWorld());
                        break;
                    case 1:
                        tempPheromonePtr = new Pheromone(IID_PHEROMONE_TYPE1, getX(), getY(), m_colony, getWorld());
                        break;
                    case 2:
                        tempPheromonePtr = new Pheromone(IID_PHEROMONE_TYPE2, getX(), getY(), m_colony, getWorld());
                        break;
                    case 3:
                        tempPheromonePtr = new Pheromone(IID_PHEROMONE_TYPE3, getX(), getY(), m_colony, getWorld());
                        break;
                    default:
                        break;
                }
                getWorld()->addActor(tempPheromonePtr);
            }
            ++m_ic;
            return;
            break;
        }
        case Compiler::faceRandomDirection:
            setDirection(Direction(randInt(1, 4)));
            ++m_ic;
            return;
            break;
        case Compiler::rotateClockwise:
            if (getDirection() == up) {
                setDirection(right);
            } else if (getDirection() == down) {
                setDirection(left);
            } else if (getDirection() == left) {
                setDirection(up);
            } else if (getDirection() == right) {
                setDirection(down);
            }
            ++m_ic;
            return;
            break;
        case Compiler::rotateCounterClockwise:
            if (getDirection() == up) {
                setDirection(left);
            } else if (getDirection() == down) {
                setDirection(right);
            } else if (getDirection() == left) {
                setDirection(down);
            } else if (getDirection() == right) {
                setDirection(up);
            }
            ++m_ic;
            return;
            break;
        case Compiler::generateRandomNumber:
            if (stoi(cmd.operand1) == 0) {
                m_lastRandomNumberGenerated = 0;
            } else {
                m_lastRandomNumberGenerated = randInt(0, (stoi(cmd.operand1)-1));
            }
            ++m_ic; // advance to next instruction
            break;
        case Compiler::goto_command:
            // just set m_ic to the specified position in operand1
            m_ic = stoi(cmd.operand1);
            break;
        case Compiler::if_command:
            switch (stoi(cmd.operand1)) {
                case Compiler::last_random_number_was_zero:
                    if (m_lastRandomNumberGenerated == 0) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                case Compiler::i_am_carrying_food:
                    if (m_foodHolding > 0) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                case Compiler::i_am_hungry:
                    if (getEnergy() <= 25) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                case Compiler::i_am_standing_with_an_enemy:
                    if (getWorld()->isActorAt(getX(), getY(), m_colony, isEnemyAt)) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                case Compiler::i_am_standing_on_food: {
                    Actor *tempFoodPtr = getWorld()->getEdibleAt(getX(), getY());
                    if (tempFoodPtr != nullptr && !tempFoodPtr->isDead()) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                }
                case Compiler::i_am_standing_on_my_anthill: {
                    if (getWorld()->isActorAt(getX(), getY(), m_colony, isAntHillAt)) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                }
                case Compiler::i_smell_pheromone_in_front_of_me: {
                    int x, y;
                    getXYInFrontOfMe(x, y, getDirection());
                    Actor *tempPheromonePtr = getWorld()->getPheromoneAt(x, y, m_colony);
                    if (tempPheromonePtr != nullptr && !tempPheromonePtr->isDead()) {
                        m_ic = stoi(cmd.operand2); //Pheromone of ant's colony found in front of it
                    } else {
                        ++m_ic;
                    }
                    break;
                }
                case Compiler::i_smell_danger_in_front_of_me:
                    int x, y;
                    getXYInFrontOfMe(x, y, getDirection());
                    if (getWorld()->isActorAt(getX(), getY(), m_colony, isDangerAt)) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                case Compiler::i_was_bit:
                    if (m_wasBitten) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                case Compiler::i_was_blocked_from_moving:
                    if (m_wasBlockedFromMoving) {
                        m_ic = stoi(cmd.operand2);
                    } else {
                        ++m_ic;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    commandsExecuted++;
    }
    return;
}


bool Ant::moveForwardIfPossible() {
    if (Insect::moveForwardIfPossible()) {
        if (getWorld()->isActorAt(getX(), getY(), 0, isPoisonAt)) {getPoisoned();}
        if (getWorld()->isActorAt(getX(), getY(), 0, isWaterAt)) {getStunned();}
        m_wasBlockedFromMoving = false;
        m_wasBitten = false;
        return true;
    }
    //Else, could not move forward
    m_wasBlockedFromMoving = true;
    return false;
}

void Ant::getBitten(int amt) {
    Insect::getBitten(amt);
    m_wasBitten = true;
}

void Grasshopper::doSomething() {
    setDoSomethingCalled(true);
    updateEnergy(getEnergy()-1);
    if (getStunnedTicks() > 0) {
        setStunnedTicks(getStunnedTicks()-1);
        return;
    }
    if (specificDoSomething()) {return;}
    if (pickupAndEatFood(200)) {
        //50% chance of rest
        if (randInt(1, 2) == 1) {
            setStunnedTicks(getStunnedTicks()+2);
            return;
        }
    }
    if (getDistanceToWalk() <= 0) {
        setDirection(Direction(randInt(1, 4)));
        setDistanceToWalk(randInt(2, 10));
    }
    moveForwardIfPossible();
    setStunnedTicks(getStunnedTicks()+2);
}

bool AdultGrasshopper::specificDoSomething() {
    if (randInt(1, 3) == 1) {
        getWorld()->biteEnemyAt(this, 0, 50); //Colony doesn't matter
        setStunnedTicks(getStunnedTicks()+2);
        return true;
    } else if (randInt(1, 10) == 1) {
        //Jump
        list<pair<int, int>> possibleJumpLocations;
        for (int i = 0; i < 360; i += 10) { //checking every 10 degrees will cover all squares
            for (int k = 10; k > 0; k--) { //check all spots from radius 1 to 10
                int cosValue = k * ( cos ( i * (4 * atan(1.0)) / 180.0 ) );
                int sinValue = k * ( sin ( i * (4 * atan(1.0)) / 180.0 ) );
                if (getX() + cosValue >= 0 && getX() + cosValue < 64 && getY() + sinValue >= 0 && getY() + sinValue < 64 && !getWorld()->isActorAt(getX() + cosValue, getY() + sinValue, 0, isPebbleAtLocation)) { //Check if its in bounds and no pebble
                    possibleJumpLocations.push_back(make_pair(cosValue, sinValue));
                }
            }
        }
        possibleJumpLocations.sort();
        possibleJumpLocations.unique(); //Delete all duplicate possible positions to randomize correctly
        int chosenRandomLocation = randInt(0, static_cast<int>(possibleJumpLocations.size())-1); //static cast to get rid of warning because .size() is unsigned long but randInt asks for an int
        list<pair<int, int>>::iterator iterator = possibleJumpLocations.begin();
        advance( iterator, chosenRandomLocation ); //Set iterator to random location
        moveTo(getX() + iterator->first, getY() + iterator->second);
        setStunnedTicks(getStunnedTicks()+2);
        return true;
    }
    return false;
}

void AdultGrasshopper::getBitten(int amt) {
    Insect::getBitten(amt);
    if (!isDead() && randInt(1, 2) == 1) {
        //50% chance it will bite back after being bitten
        getWorld()->biteEnemyAt(this, 0, 50); //Colony doesn't matter
    }
}

bool BabyGrasshopper::specificDoSomething() {
    if (getEnergy() >= 1600) {
        //Transform into adult grasshopper
        Actor *tempAdultGrasshopperPtr = new AdultGrasshopper(IID_ADULT_GRASSHOPPER, getX(), getY(), getWorld());
        getWorld()->addActor(tempAdultGrasshopperPtr);
        updateEnergy(0);
        return true;
    }
    return false;
}

bool BabyGrasshopper::moveForwardIfPossible() {
    if (Insect::moveForwardIfPossible()) {
        if (getWorld()->isActorAt(getX(), getY(), 0, isPoisonAt)) {getPoisoned();}
        if (getWorld()->isActorAt(getX(), getY(), 0, isWaterAt)) {getStunned();}
        return true;
    }
    return false;
}
