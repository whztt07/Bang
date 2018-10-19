#ifndef PARTICLE_H
#define PARTICLE_H

#include <functional>

#include "Bang/Array.h"
#include "Bang/Array.tcc"
#include "Bang/BangDefines.h"
#include "Bang/Color.h"
#include "Bang/Math.h"
#include "Bang/Vector2.h"
#include "Bang/Vector3.h"

namespace Bang
{
class Collider;
class Time;

class Particle
{
public:
    enum class PhysicsStepMode
    {
        EULER,
        EULER_SEMI,
        VERLET
    };

    struct Data
    {
        Vector3 prevPosition = Vector3::Zero;
        Vector3 position = Vector3::Zero;
        Vector3 velocity = Vector3::Zero;
        Vector3 force = Vector3::Zero;
        Color startColor = Color::White;
        Color endColor = Color::White;
        Color currentColor = Color::White;
        float totalLifeTime = Math::Infinity<float>();
        float remainingLifeTime = Math::Infinity<float>();
        float remainingStartTime = 0.0f;
        float prevDeltaTimeSecs = 1.0f;
        float size = 1.0f;
        uint currentFrame = 0;
    };

    struct Parameters
    {
        float damping = 1.0f;
        float bounciness = 1.0f;
        Array<Collider *> colliders;
        bool computeCollisions = false;
        Vector3 gravity = Vector3::Zero;
        Particle::PhysicsStepMode physicsStepMode = PhysicsStepMode::EULER;

        float animationSpeed = 0.0f;
        Vector2i animationSheetSize = Vector2i::One;
    };

    static void ExecuteFixedStepped(
        Time totalDeltaTime,
        Time fixedStepDeltaTime,
        std::function<void(Time fixedDeltaTime)> func);

    static void FixedStepAll(
        Array<Particle::Data> *particlesDatas,
        Time totalDeltaTime,
        Time fixedStepDeltaTime,
        const Particle::Parameters &params,
        std::function<void(uint, const Particle::Parameters &)>
            initParticleFunc);

    static void FixedStepAll(
        Array<Particle::Data> *particlesDatas,
        Time totalDeltaTime,
        Time fixedStepDeltaTime,
        const Particle::Parameters &params,
        std::function<void(uint, const Particle::Parameters &)>
            initParticleFunc,
        std::function<bool(uint)> canUpdateParticleFunc);

    static void FixedStepAll(
        Array<Particle::Data> *particlesDatas,
        Time totalDeltaTime,
        Time fixedStepDeltaTime,
        const Particle::Parameters &params,
        std::function<void(uint i, const Particle::Parameters &)>
            initParticleFunc,
        std::function<bool(uint i)> canUpdateParticleFunc,
        std::function<void(Time dt)> extraFuncToExecuteEveryStep);

    static void Step(Particle::Data *pData,
                     Time dt,
                     const Particle::Parameters &params);

    Particle() = delete;
    virtual ~Particle() = delete;

private:
    static void StepPositionAndVelocity(Particle::Data *pData,
                                        float dt,
                                        const Particle::Parameters &params);
    static bool CollideParticle(Collider *collider,
                                const Parameters &params,
                                const Vector3 &prevPositionNoInt,
                                const Vector3 &newPositionNoInt,
                                const Vector3 &newVelocityNoInt,
                                Vector3 *newPositionAfterInt,
                                Vector3 *newVelocityAfterInt);
};
}

#endif  // PARTICLE_H