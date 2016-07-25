#ifndef WORLD_H
#define WORLD_H

#include <map>
#include "ChronoMessages.pb.h"

class World
{
public:
    World();
    ~World();
    
private:
    std::map <int, std::map <int, ChronoMessages::VehicleMessage>> sectionMap;
};

#endif // WORLD_H
