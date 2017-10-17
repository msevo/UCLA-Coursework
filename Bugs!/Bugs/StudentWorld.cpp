#include "StudentWorld.h"
#include <string>
#include <iostream> // defines the overloads of the << operator
#include <sstream>  // defines the type std::ostringstream
#include <iomanip>  // defines the manipulator setw
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

int StudentWorld::init()
{
    
    Field field;
    string fieldFile = getFieldFilename();
    
    string error;
    if (field.loadField(fieldFile, error) != Field::LoadResult::load_success)
    {
        setError(fieldFile + " " + error);
        return GWSTATUS_LEVEL_ERROR; // something bad happened!
    }
    std::vector<std::string> fileNames = getFilenamesOfAntPrograms();
    antCompilers = static_cast<int>(fileNames.size()); //static cast to get rid of warning because .size() is unsigned long but antCompilers asks for an int
    for (int i = 0; i < antCompilers; i++) {
        m_colonyScores[i] = 0;
        m_comp[i] = new Compiler;
        string error2;
        // compile the source file... If the compile function returns false, there was a syntax error during compilation!
        if (!m_comp[i]->compile(fileNames[i], error2)) {
            setError(fileNames[i] + " " + error2);
            return GWSTATUS_LEVEL_ERROR;
        }
    }
    
    for (int i = 0; i < VIEW_HEIGHT; i++) {
        for (int j = 0; j < VIEW_WIDTH; j++) {
            Field::FieldItem item = field.getContentsOf(j,i); // note it’s x,y and not y,x!!!
            if (item == Field::FieldItem::rock) {
                Actor *tempActor = new Pebble(IID_ROCK, j, i, this);
                m_actorList[make_pair(j, i)].push_back(tempActor);
            } else if (item == Field::FieldItem::food) {
                Actor *tempActor = new Food(IID_FOOD, j, i, 6000, this);
                m_actorList[make_pair(j, i)].push_back(tempActor);
            } else if (item == Field::FieldItem::anthill0) {
                if (antCompilers >= 1) {
                    Actor *tempActor = new AntHill(IID_ANT_HILL, j, i, 0, m_comp[0], this);
                    m_actorList[make_pair(j, i)].push_back(tempActor);
                }
            } else if (item == Field::FieldItem::anthill1) {
                if (antCompilers >= 2) {
                    Actor *tempActor = new AntHill(IID_ANT_HILL, j, i, 1, m_comp[1], this);
                    m_actorList[make_pair(j, i)].push_back(tempActor);
                }
            } else if (item == Field::FieldItem::anthill2) {
                if (antCompilers >= 3) {
                    Actor *tempActor = new AntHill(IID_ANT_HILL, j, i, 2, m_comp[2], this);
                    m_actorList[make_pair(j, i)].push_back(tempActor);
                }
            } else if (item == Field::FieldItem::anthill3) {
                if (antCompilers >= 4) {
                    Actor *tempActor = new AntHill(IID_ANT_HILL, j, i, 3, m_comp[3], this);
                    m_actorList[make_pair(j, i)].push_back(tempActor);
                }
            } else if (item == Field::FieldItem::water) {
                Actor *tempActor = new WaterPool(IID_WATER_POOL, j, i, this);
                m_actorList[make_pair(j, i)].push_back(tempActor);
            } else if (item == Field::FieldItem::poison) {
                Actor *tempActor = new Poison(IID_POISON, j, i, this);
                m_actorList[make_pair(j, i)].push_back(tempActor);
            } else if (item == Field::FieldItem::grasshopper) {
                Actor *tempActor = new BabyGrasshopper(IID_BABY_GRASSHOPPER, j, i, this);
                m_actorList[make_pair(j, i)].push_back(tempActor);
            }
        }
    }
    
    m_tickCount = 0;
    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
    
    resetActors();
    callActorsActions();
    removeDeadActors();
    
    setDisplayText();
    
    m_tickCount++;
    if (m_tickCount >= 2000) {
        if (antCompilers == 0) { //No ant programs
            return GWSTATUS_NO_WINNER;
        } else if (m_colonyScores[m_winningAntNumber] == 5) { //No colony made more ants than original 5, so no colony wins
            return GWSTATUS_NO_WINNER;
        } else {
            setWinner(m_comp[m_winningAntNumber]->getColonyName());
            return GWSTATUS_PLAYER_WON;
        }
    }
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    map<pair<int, int>, list<Actor*>>::iterator i;
    for (i = m_actorList.begin(); i != m_actorList.end(); i++) {
        while (!i->second.empty()) {
            delete i->second.front();
            i->second.pop_front();
        }
    }
}

void StudentWorld::setDisplayText() {
    string s = formatDisplayText(m_winningAntNumber);
    setGameStatText(s);
}

string StudentWorld::formatDisplayText(int winningAnt) {
    ostringstream oss;
    oss << "Ticks:";
    oss << setw(5) << 2000-m_tickCount << " -";
    for (int i = 0; i < antCompilers; i++) {
        oss << "  " << m_comp[i]->getColonyName();
        if (i == winningAnt && m_colonyScores[i] != 5) {oss << "*";}
        oss << ": ";
        oss.fill('0');
        oss << setw(2) << m_colonyScores[i];
    }
    string s = oss.str();
    return s;
}

void StudentWorld::resetActors() {
    map<pair<int, int>, list<Actor*>>::iterator i1;
    for (i1 = m_actorList.begin(); i1 != m_actorList.end(); i1++) {
        list<Actor*>::iterator i2 = i1->second.begin();
        while (i2 != i1->second.end()) {
            (*i2)->setDoSomethingCalled(false);
            i2++;
        }
    }
}

void StudentWorld::callActorsActions() {
    map<pair<int, int>, list<Actor*>>::iterator i1;
    for (i1 = m_actorList.begin(); i1 != m_actorList.end(); i1++) {
        list<Actor*>::iterator i2 = i1->second.begin();
        while (i2 != i1->second.end()) {
            if (!(*i2)->getDoSomethingCalled()) {
                int oldX = (*i2)->getX(), oldY = (*i2)->getY();
                (*i2)->doSomething();
                int newX = (*i2)->getX(), newY = (*i2)->getY();
                
                if (oldX != newX || oldY != newY) {
                    m_actorList[make_pair(newX, newY)].push_back(*i2);
                    i2 = m_actorList[make_pair(oldX, oldY)].erase(i2);
                } else {
                    i2++;
                }
            } else {
                i2++;
            }
        }
    }
}

void StudentWorld::removeDeadActors() {
    map<pair<int, int>, list<Actor*>>::iterator i1;
    for (i1 = m_actorList.begin(); i1 != m_actorList.end(); i1++) {
        list<Actor*>::iterator i2 = i1->second.begin();
        while (i2 != i1->second.end()) {
            if ((*i2)->isDead()) {
                list<Actor*>::iterator i3 = i2;
                i2 = m_actorList[make_pair((*i2)->getX(), (*i2)->getY())].erase(i2);
                delete (*i3);
            } else {
                i2++;
            }
        }
    }
}

Actor* StudentWorld::getEdibleAt(int x, int y) const {
    map<pair<int, int>, list<Actor*>>::const_iterator i1 = m_actorList.find(make_pair(x, y));
    if (i1 != m_actorList.end()) {
        list<Actor*>::const_iterator tempIterator = i1->second.begin();
        while (tempIterator != m_actorList.at(make_pair(x, y)).end()) {
            if((*tempIterator)->isEdible()) {
                return *tempIterator;
            }
            tempIterator++;
        }
    }
    return nullptr;
}

Actor* StudentWorld::getPheromoneAt(int x, int y, int colony) const {
    map<pair<int, int>, list<Actor*>>::const_iterator i1 = m_actorList.find(make_pair(x, y));
    if (i1 != m_actorList.end()) {
        list<Actor*>::const_iterator tempIterator = i1->second.begin();
        while (tempIterator != m_actorList.at(make_pair(x, y)).end()) {
            if((*tempIterator)->isPheromone(colony)) {
                return *tempIterator;
            }
            tempIterator++;
        }
    }
    return nullptr;
}

bool StudentWorld::isActorAt(int x, int y, int colony, int whichFunction) const {
    map<pair<int, int>, list<Actor*>>::const_iterator i1 = m_actorList.find(make_pair(x, y));
    if (i1 != m_actorList.end()) {
        list<Actor*>::const_iterator tempIterator = i1->second.begin();
        while (tempIterator != m_actorList.at(make_pair(x, y)).end()) {
            if(whichActorAtFunction(whichFunction, tempIterator, colony)) {
                return true;
            }
            tempIterator++;
        }
    }
    return false;
}

bool StudentWorld::whichActorAtFunction(int whichFunction, list<Actor*>::const_iterator tempIterator, int colony) const {
    switch (whichFunction) {
        case 0:
            return isPoisonAt(tempIterator, colony);
            break;
        case 1:
            return isWaterAt(tempIterator, colony);
            break;
        case 2:
            return isPebbleAtLocation(tempIterator, colony);
            break;
        case 3:
            return isEnemyAt(tempIterator, colony);
            break;
        case 4:
            return isDangerAt(tempIterator, colony);
            break;
        case 5:
            return isAntHillAt(tempIterator, colony);
            break;
        default:
            break;
    }
    return false;
}

bool StudentWorld::isPoisonAt(list<Actor*>::const_iterator tempIterator, int colony) const {
    return (*tempIterator)->isPoison();
}

bool StudentWorld::isWaterAt(list<Actor*>::const_iterator tempIterator, int colony) const {
    return (*tempIterator)->isWater();
}

bool StudentWorld::isPebbleAtLocation(list<Actor*>::const_iterator tempIterator, int colony) const {
    return (*tempIterator)->blocksMovement();
}

bool StudentWorld::isEnemyAt(list<Actor*>::const_iterator tempIterator, int colony) const {
    return (*tempIterator)->isEnemy(colony);
}

bool StudentWorld::isDangerAt(list<Actor*>::const_iterator tempIterator, int colony) const {
    return (*tempIterator)->isDangerous(colony);
}

bool StudentWorld::isAntHillAt(list<Actor*>::const_iterator tempIterator, int colony) const {
    return ((*tempIterator)->isAntHill(colony) && !(*tempIterator)->isDead());
}

void StudentWorld::addActor(Actor* a) {
    m_actorList[make_pair(a->getX(), a->getY())].push_back(a);
}

void StudentWorld::increaseScore(int colony) {
    switch (colony) {
        case 0:
            m_colonyScores[colony]++;
            break;
        case 1:
            m_colonyScores[colony]++;
            break;
        case 2:
            m_colonyScores[colony]++;
            break;
        case 3:
            m_colonyScores[colony]++;
            break;
        default:
            break;
    }
    for (int i = 0; i < antCompilers; i++) {
        if(m_colonyScores[i] > m_colonyScores[m_winningAntNumber]) { //This keeps the winner as the first one to get the highest score even in tie situation
            m_winningAntNumber = i;
        }
    }
}

bool StudentWorld::biteEnemyAt(Actor* me, int colony, int biteDamage) {
    map<pair<int, int>, list<Actor*>>::const_iterator i1 = m_actorList.find(make_pair(me->getX(), me->getY()));
    if (i1 != m_actorList.end()) {
        list<Actor*>::const_iterator tempIterator = i1->second.begin();
        while (tempIterator != m_actorList.at(make_pair(me->getX(), me->getY())).end()) {
            if((*tempIterator)->isEnemy(colony) && (*tempIterator) != me) {
                (*tempIterator)->getBitten(biteDamage);
                return true;
            }
            tempIterator++;
        }
    }
    return false;
}
