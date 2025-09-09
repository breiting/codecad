#pragma once
#include <memory>
#include <pure/PureController.hpp>
#include <pure/PureScene.hpp>
#include <string>

#include "Scenario.hpp"
#include "pure/PureMeasurement.hpp"
#include "pure/PurePicker.hpp"

/**
 * @brief Tiny harness around PURE to run small OCCT scenarios.
 *
 * Controls:
 *  - 1 / 2 / 3 ... : switch scenarios
 *  - R             : rebuild current scenario
 *  - W             : toggle wireframe
 *  - Esc           : quit
 */
class App {
   public:
    App();
    bool Initialize(int width, int height, const std::string& title);
    void Run();
    void Shutdown();

    void AddScenario(const ScenarioPtr& s);

   private:
    void Rebuild();
    void SaveSTL();
    void SwitchTo(size_t index);

   private:
    std::unique_ptr<pure::PureController> m_Controller;
    std::shared_ptr<pure::PureScene> m_Scene;

    std::unique_ptr<pure::PurePicker> m_Picker;
    pure::PureMeasurement m_Measure;

    std::vector<ScenarioPtr> m_Scenarios;
    size_t m_Current = 0;
};
