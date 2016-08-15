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
  ServerVehicle(ChSystem* system);
  ~ServerVehicle();

  ChBody& GetChassis();
  void update(ChronoMessages::VehicleMessage&);

 private:
  ChSystem* m_system;
  std::shared_ptr<ChBody> m_chassis;
  std::shared_ptr<ChBodyEasyBox> m_hitbox;
  std::vector<std::shared_ptr<ChBody>> m_wheels;
};

#endif  // SERVERVEHICLE_H
