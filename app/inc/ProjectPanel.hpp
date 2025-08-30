#pragma once
#include <chrono>
#include <functional>
#include <io/Project.hpp>

class ProjectPanel {
   public:
    using SaveCallback = std::function<void(const io::Project&)>;

    explicit ProjectPanel(io::Project& project);
    void SetOnSave(SaveCallback cb);
    void Draw();
    void ForceSaveNow();

   private:
    bool DrawMeta(io::Meta& meta);
    bool DrawParams(io::ParamsMap& params);

   private:
    io::Project& proj;
    SaveCallback onSave;
    bool dirty = false;
    std::chrono::steady_clock::time_point lastEdit = std::chrono::steady_clock::now();
    const int debounceMs = 300;  // Save after debounce
};
