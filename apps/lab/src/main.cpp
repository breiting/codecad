#include "App.hpp"
#include "CupScenario.hpp"
#include "FilletScenario.hpp"
#include "ThreadScenario.hpp"

int main() {
    App app;
    if (!app.Initialize(1600, 1200, "CodeCAD Lab Prototyping")) return 1;

    app.AddScenario(std::make_shared<FilletScenario>());
    app.AddScenario(std::make_shared<ThreadScenario>());
    app.AddScenario(std::make_shared<CupScenario>());

    app.Run();
    app.Shutdown();
    return 0;
}
