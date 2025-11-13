#pragma once
#include <string>

#include "Scenario.hpp"

/**
 * @brief Simple poisson disk scenario
 */
class PoissonScenario : public Scenario {
   public:
    std::string Name() const override {
        return "Poisson Testing";
    }
    void Build(std::shared_ptr<pure::PureScene> scene) override;
};
