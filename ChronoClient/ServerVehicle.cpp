#include "ServerVehicle.h"
#include "chrono/assets/ChTexture.h"
#define VEH_NUM_WHEELS 4

using namespace chrono;

ServerVehicle::ServerVehicle(ChSystem* system) {
    std::cout << "Creating new ServerVehicle..." << std::endl;
    m_chassis = std::make_shared<ChBody>();
    m_chassis->SetBodyFixed(true);
    m_hitbox = std::make_shared<ChBodyEasyBox>(4.55, 2.2, 1.3, 3000, true, true);
    m_hitbox->SetBodyFixed(true);
    m_hitbox->SetCollide(true);
    m_hitbox->SetPos(ChVector<>(0, 0, 1));

    for (int i = 0; i < VEH_NUM_WHEELS; i++) {
        std::shared_ptr<ChBody> wheel = std::make_shared<ChBody>();
        wheel->SetBodyFixed(true);
        auto cyl = std::make_shared<ChCylinderShape>();
        cyl->GetCylinderGeometry().rad = 0.0254 * 18.15;
        cyl->GetCylinderGeometry().p1 = ChVector<>(0, 0.0254 * 10 / 2, 0);
        cyl->GetCylinderGeometry().p2 = ChVector<>(0, -0.0254 * 10 / 2, 0);
        wheel->AddAsset(cyl);

        auto tex = std::make_shared<ChTexture>();
        tex->SetTextureFilename(GetChronoDataFile("bluwhite.png"));
        wheel->AddAsset(tex);
        m_wheels.push_back(wheel);
        system->Add(wheel);
    }

    auto sphere = std::make_shared<ChSphereShape>();
    sphere->GetSphereGeometry().rad = 0.1;

    sphere->Pos = ChVector<>(0.055765, 0, 0.52349);

    m_chassis->AddAsset(sphere);

    auto tex = std::make_shared<ChTexture>();
    tex->SetTextureFilename(GetChronoDataFile("pinkwhite.png"));
    m_chassis->AddAsset(tex);
    system->Add(m_chassis);
    system->Add(m_hitbox);
    m_system = system;
    std::cout << "server vehicle created." << std::endl;
}

ServerVehicle::~ServerVehicle() {
  std::cout << "Destroying ServerVehicle" << std::endl;
  for(std::shared_ptr<ChBody> wheel : m_wheels)
      m_system->RemoveBody(wheel);
  m_system->RemoveBody(m_hitbox);
  m_system->RemoveBody(m_chassis);
}

ChBody& ServerVehicle::GetChassis() {
    return *m_chassis;
}

void ServerVehicle::update(ChronoMessages::VehicleMessage& message) {
  m_chassis->SetPos(ChVector<>(message.chassiscom().x(),
                               message.chassiscom().y(),
                               message.chassiscom().z()));
  m_hitbox->SetPos(ChVector<>(message.chassiscom().x(),
                              message.chassiscom().y(),
                              message.chassiscom().z()) +
                   ChVector<>(0, .1, .5));
  m_chassis->SetRot(
      ChQuaternion<>(message.chassisrot().e0(), message.chassisrot().e1(),
                     message.chassisrot().e2(), message.chassisrot().e3()));
  m_hitbox->SetRot(
      ChQuaternion<>(message.chassisrot().e0(), message.chassisrot().e1(),
                     message.chassisrot().e2(), message.chassisrot().e3()));

  m_wheels.at(0)->SetPos(ChVector<>(message.backleftwheelcom().x(),
                                    message.backleftwheelcom().y(),
                                    message.backleftwheelcom().z()));
  m_wheels.at(1)->SetPos(ChVector<>(message.backrightwheelcom().x(),
                                    message.backrightwheelcom().y(),
                                    message.backrightwheelcom().z()));
  m_wheels.at(2)->SetPos(ChVector<>(message.frontleftwheelcom().x(),
                                    message.frontleftwheelcom().y(),
                                    message.frontleftwheelcom().z()));
  m_wheels.at(3)->SetPos(ChVector<>(message.frontrightwheelcom().x(),
                                    message.frontrightwheelcom().y(),
                                    message.frontrightwheelcom().z()));

  m_wheels.at(0)->SetRot(ChQuaternion<>(
      message.backleftwheelrot().e0(), message.backleftwheelrot().e1(),
      message.backleftwheelrot().e2(), message.backleftwheelrot().e3()));
  m_wheels.at(1)->SetRot(ChQuaternion<>(
      message.backrightwheelrot().e0(), message.backrightwheelrot().e1(),
      message.backrightwheelrot().e2(), message.backrightwheelrot().e3()));
  m_wheels.at(2)->SetRot(ChQuaternion<>(
      message.frontleftwheelrot().e0(), message.frontleftwheelrot().e1(),
      message.frontleftwheelrot().e2(), message.frontleftwheelrot().e3()));
  m_wheels.at(3)->SetRot(ChQuaternion<>(
      message.frontrightwheelrot().e0(), message.frontrightwheelrot().e1(),
      message.frontrightwheelrot().e2(), message.frontrightwheelrot().e3()));
}
