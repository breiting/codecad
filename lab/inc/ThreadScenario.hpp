#pragma once
#include <string>

#include "Scenario.hpp"

/**
 * @brief Simple thread demo: Printing an ISO bolt and nut
 */
class ThreadScenario : public Scenario {
   public:
    std::string Name() const override {
        return "ISO Thread";
    }
    void Build(std::shared_ptr<pure::PureScene> scene) override;
};
