#pragma once

#include <ccad/base/Shape.hpp>
#include <ccad/mech/Threads.hpp>
namespace ccad {
namespace mech {

struct RodSpec {
    double diameter = 10;
    double length = 200.0;
};

/// \brief Make a simple rod
Shape Rod(const RodSpec& spec);

/// \brief Make a threaded rod
Shape ThreadedRod(const RodSpec& rodSpec, const ThreadSpec& threadSpec);
}  // namespace mech
}  // namespace ccad
