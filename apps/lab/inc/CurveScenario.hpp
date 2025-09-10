#pragma once
#include <string>

#include "Scenario.hpp"

/**
 * @brief Simple curve scenario
 */
class CurveScenario : public Scenario {
   public:
    std::string Name() const override {
        return "Curve Testing";
    }
    void Build(std::shared_ptr<pure::PureScene> scene) override;
};
