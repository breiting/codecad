/** To change the standard LogLevel in any function you want to debug/log use
 ** the follwoing syntax : LogSettings::getInstance()->setLogLevel(
 **                          LogSettings::getInstance()->logLevel() & (uint_16t)<LOGLEVEL>)
 ** where <LOGLEVEL> matches one of the following levels:
 ** enum class LoggerLevels : uint16_t {
 **   NONE_ = 0x00,
 **   INFO_ = 0x01,
 **   DEBUG_ = 0x02,
 **   WARN_ = 0x04,
 **   ERROR_ = 0x08
 ** };
 **/

#include <ccad/base/Logger.hpp>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <mutex>

using namespace std::chrono;
using namespace ccad;

namespace {
std::atomic<LogSettings *> m_instance;
std::mutex m_mutex;
}  // namespace

Log::Log(uint16_t level)
    :  // Do that for safety reasons - customer shall not be able to turn logging on!
      m_output((LogSettings::getInstance()->logLevel() & level) > 0),
      m_endlLast(false) {
    if (m_output) {
        std::lock_guard<std::mutex> lock(m_acquisitionLock);
        LogSettings::getInstance()->stream()
            << "[" << LogSettings::getInstance()->getLogTime("%Y/%m/%d %X").c_str() << "]";
    }
}

Log::~Log() {
    if (!m_endlLast && m_output) {
        std::lock_guard<std::mutex> lock(m_acquisitionLock);
        LogSettings::getInstance()->stream() << std::endl;
    }
}

Log &Log::operator<<(EndLine endlFunc) {
    if (m_output) {
        // invoke
        m_endlLast = true;
        std::lock_guard<std::mutex> lock(m_acquisitionLock);
        endlFunc(LogSettings::getInstance()->stream());
    }
    return *this;
}

LogSettings *LogSettings::getInstance() {
    LogSettings *tmp = m_instance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (tmp == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        tmp = m_instance.load(std::memory_order_relaxed);
        if (tmp == nullptr) {
            tmp = new LogSettings;
            std::atomic_thread_fence(std::memory_order_release);
            m_instance.store(tmp, std::memory_order_relaxed);
        }
    }
    return tmp;
}

LogSettings::LogSettings() : m_logSetting(0) {
    m_logSetting |= (uint16_t)LoggerLevels::INFO_;
    m_logSetting |= (uint16_t)LoggerLevels::DEBUG_;
    m_logSetting |= (uint16_t)LoggerLevels::WARN_;
    m_logSetting |= (uint16_t)LoggerLevels::ERROR_;
}

LogSettings::~LogSettings() {
    if (fout.is_open()) {
        fout.close();
    }
}

void LogSettings::setLogLevel(uint16_t logSetting) {
    m_logSetting = logSetting;
}

uint16_t LogSettings::logLevel() const {
    return m_logSetting;
}

const std::string LogSettings::getLogTime(const std::string &dateTimeFormat) const {
    time_t mytime;
    mytime = time(NULL);

    char mbstr[100];
    memset(&mbstr, 0, sizeof(mbstr));  // initialize to 0
    std::strftime(mbstr, sizeof(mbstr), dateTimeFormat.c_str(), std::localtime(&mytime));
    return mbstr;
}

void LogSettings::setLogFile(std::string logFilePath) {
    // Write to File
    if (fout.is_open()) {
        fout.close();
    }

    if (logFilePath.length() > 0) {
        fout.open(logFilePath.c_str(), std::ios::out);
        if (!fout.is_open()) {
            std::cerr << "error: open file for output failed!" << std::endl;
        }
    }
}

std::ostream &LogSettings::stream() {
    if (fout.is_open()) {
        return fout;
    } else {
        return std::cout;
    }
}
