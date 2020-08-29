#include "scenario_physics.h"

#include <reactphysics3d/reactphysics3d.h>
using namespace reactphysics3d;

PhysicsScenario::PhysicsScenario() {
  _physicsCommon = new PhysicsCommon();

  // Create a physics world 
  _physicsWorld = _physicsCommon->createPhysicsWorld();

  // // Create a rigid body in the world 
  // Vector3 position(0, 20, 0); 
  // Quaternion orientation = Quaternion::identity(); 
  // Transform transform(position, orientation); 
  // RigidBody* body = _physicsWorld->createRigidBody(transform); 

  // const decimal timeStep = 1.0f / 60.0f; 

  // // Step the simulation a few steps 
  // for (int i=0; i < 20; i++) { 

  //     _physicsWorld->update(timeStep); 

  //     // Get the updated position of the body 
  //     const Transform& transform = body->getTransform(); 
  //     const Vector3& position = transform.getPosition(); 

  //     // Display the position of the body 
  //     std::cout << "Body Position: (" << position.x << ", " << 
  //   position.y << ", " << position.z << ")" << std::endl; 
  // }
}

bool PhysicsScenario::finishLoading() {
  return true;
}

void PhysicsScenario::think(glm::vec2 dir, double pitch, double yaw) {

}
void PhysicsScenario::render() {

}