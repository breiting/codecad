#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>

/** Logging macro; to overcome multi macro definitions for levels
 * To create a Logentry which should be put out in two ore more cases use the follwing syntax:
 * Log((uint16_t)LoggerLevels::WARN_|(uint16_t)LoggerLevels::ERROR_)) << "LogMessage";
 */
#define LOG(X) ccad::Log((uint16_t)LoggerLevels::X##_) << " " << #X << " "

/** Definition of LogLevels
 * Loglevels are used as binary bitmask format an can be used together by using binary or
 * The set loglevel is binary and compared to the level required for the output
 */
enum class LoggerLevels : uint16_t { NONE_ = 0x00, INFO_ = 0x01, DEBUG_ = 0x02, WARN_ = 0x04, ERROR_ = 0x08 };

namespace ccad {
class LogSettings {
   public:
    /// Creates a global available instance (even creation is thread safe)
    static LogSettings *getInstance();

    /// Sets global Loglevel (there is a default definition of WARN_ and ERROR_)
    void setLogLevel(uint16_t logLevel);

    /// Can be used to pipe logging to a logfile (has to be called explicitly)
    void setLogFile(std::string logfilePath);

    /// Returns the configured loglevel (the one from the class which defines what messages shall be created)
    uint16_t logLevel() const;

    /// Returns the current stream (might be "cout" or a file-stream if a logfile was specified before
    std::ostream &stream();

    /// Returns the current LogTime (current time according to the given format)
    const std::string getLogTime(const std::string &dateTimeFormat) const;

    /// Explicitly define assignment operator as deleted, so prohibit copying
    LogSettings &operator=(const LogSettings &) = delete;

   private:
    LogSettings();

    /// Explicitly define copy ctor as deleted, so prohibit copying
    LogSettings(const LogSettings &) = delete;
    ~LogSettings();

    uint16_t m_logSetting;
    std::ofstream fout;
};

class Log {
   public:
    // this is the signature for the std::endl function
    using EndLine =
        std::basic_ostream<char, std::char_traits<char> > &(*)(std::basic_ostream<char, std::char_traits<char> > &);

    Log(uint16_t level = 0);
    virtual ~Log();

    // default output
    template <typename T>
    Log &operator<<(const T &x) {
        if (m_output) {
            m_endlLast = false;
            std::lock_guard<std::mutex> lock(m_acquisitionLock);
            LogSettings::getInstance()->stream() << x;
        }
        return *this;
    }

    // define an operator<< to take in std::endl
    Log &operator<<(EndLine endlFunc);

   private:
    bool m_output;
    bool m_endlLast;
    std::mutex m_acquisitionLock;
};
}  // namespace ccad
