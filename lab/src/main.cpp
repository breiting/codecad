#include "App.hpp"
#include "ThreadScenario.hpp"

int main() {
    App app;
    if (!app.Initialize(1280, 800, "ccad-lab")) return 1;

    app.AddScenario(std::make_shared<ThreadScenario>());

    app.Run();
    app.Shutdown();
    return 0;
}
