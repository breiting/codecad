#pragma once
#include <chrono>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

class FileWatcher {
   public:
    using Clock = std::chrono::steady_clock;

    FileWatcher(const std::string& path, std::chrono::milliseconds interval = std::chrono::milliseconds(300))
        : m_Path(path), m_Interval(interval) {
        Touch();
    }

    void Poll(const std::function<void(const std::string&)>& onChange) {
        auto now = Clock::now();
        if (now - m_LastCheck < m_Interval) return;
        m_LastCheck = now;

        try {
            auto mtime = std::filesystem::last_write_time(m_Path);
            if (!m_LastWrite || mtime != *m_LastWrite) {
                m_LastWrite = mtime;
                onChange(m_Path);
            }
        } catch (...) {
            // Currently ignoring
        }
    }

    void Touch() {
        try {
            m_LastWrite = std::filesystem::last_write_time(m_Path);
        } catch (...) {
            m_LastWrite.reset();
        }
        m_LastCheck = Clock::now();
    }

   private:
    std::string m_Path;
    std::chrono::milliseconds m_Interval;
    Clock::time_point m_LastCheck{};
    std::optional<std::filesystem::file_time_type> m_LastWrite;
};
