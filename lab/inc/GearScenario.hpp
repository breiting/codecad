#pragma once
#include <string>

#include "Scenario.hpp"

/**
 * @brief Simple gear demo
 *
 */
class GearScenario : public Scenario {
   public:
    std::string Name() const override {
        return "Gear Demo";
    }
    void Build(std::shared_ptr<pure::PureScene> scene) override;
};
