#ifndef SERVERVEHICLE_H
#define SERVERVEHICLE_H

#include <memory>
#include <vector>
#include "ChronoMessages.pb.h"
#include "chrono/physics/ChSystem.h"
#include "chrono/utils/ChUtilsInputOutput.h"
#include "physics/ChBodyEasy.h"

using namespace chrono;

class ServerVehicle {
 public:
  ServerVehicle(ChSystem&);
  //  ServerVehicle(ChSystem&, ChronoMessages::VehicleMessage&);
  std::shared_ptr<ChBody> m_chassis;
  std::shared_ptr<ChBodyEasyBox> m_hitbox;
  std::vector<std::shared_ptr<ChBody>> m_wheels;
  //  int m_id;
  void update(ChronoMessages::VehicleMessage&);
};

#endif  // SERVERVEHICLE_H
