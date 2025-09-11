#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

class Time {
   public:
    static std::string GetCurrentDateTimeString() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);

        std::tm local_tm;
        localtime_r(&t, &local_tm);
        std::ostringstream oss;
        oss << std::put_time(&local_tm, "%Y-%m-%d_%H_%M");
        return oss.str();
    }
};
