#pragma once

class RigidBody;
class PhysicsWorld;

class PhysicsComponent {
   public:
    virtual ~PhysicsComponent() = default;

    virtual void Update(float deltaTime) = 0;
    virtual void ApplyToPhysicsWorld(PhysicsWorld& world) = 0;
};
