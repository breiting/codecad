#pragma once
#include <string>

#include "Scenario.hpp"

/**
 * @brief Simple coarse/external thread demo: a chunky one-turn bolt and matching nut block.
 *
 * Parameters are intentionally big and simple to visualize boolean stability.
 * You can tweak them in Build() for experiments.
 */
class CupScenario : public Scenario {
   public:
    std::string Name() const override {
        return "Dose Demo";
    }
    void Build(std::shared_ptr<pure::PureScene> scene) override;
};
