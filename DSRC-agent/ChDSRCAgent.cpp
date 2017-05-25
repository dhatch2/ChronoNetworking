#include <iostream>
#include "core/ChVector.h"
#include "ChDSRCAgent.h"

ChDSRCAgent::ChDSRCAgent(std::shared_ptr<HMMWV_Full> veh) {
    vehicle = veh;
}
