#pragma once
#include <Project.hpp>
#include <chrono>
#include <functional>

class ProjectPanel {
   public:
    using SaveCallback = std::function<void(const Project&)>;

    explicit ProjectPanel(Project& project);
    void SetOnSave(SaveCallback cb);
    void Draw();
    void ForceSaveNow();

   private:
    bool DrawMeta(Meta& meta);
    bool DrawParams(ParamsMap& params);

   private:
    Project& m_Project;
    SaveCallback m_OnSave;
    bool m_Dirty = false;
    std::chrono::steady_clock::time_point m_LastEdit = std::chrono::steady_clock::now();
    const int m_DebounceMs = 300;  // Save after debounce
};
