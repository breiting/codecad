#pragma once

#include <ccad/base/Shape.hpp>
#include <ccad/mech/Threads.hpp>
namespace ccad {
namespace mech {

struct RodSpec {
    bool chamferBottom = false;
    bool chamferTop = false;
};

/// \brief Make a simple rod
Shape Rod(double diameter, double length, const RodSpec& spec);

/// \brief Make a threaded rod
Shape ThreadedRod(double totalLength, double threadLength, const RodSpec& rodSpec, const ThreadSpec& threadSpec);
}  // namespace mech
}  // namespace ccad
