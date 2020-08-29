#ifndef SCENARIO_PHYSICS_H
#define SCENARIO_PHYSICS_H

#include "scenario.h"

namespace reactphysics3d {
struct PhysicsCommon;
struct PhysicsWorld;
}

struct PhysicsScenario : IScenario {
public:
  PhysicsScenario();
  void think(glm::vec2 dir, double pitch, double yaw) override;
  void render() override;

private:
  bool finishLoading() override;

  reactphysics3d::PhysicsCommon* _physicsCommon;
  reactphysics3d::PhysicsWorld* _physicsWorld;
};

#endif
