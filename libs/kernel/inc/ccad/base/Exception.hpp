#pragma once

#include <ccad/base/Status.hpp>
#include <string>

namespace ccad {
class Exception : public std::exception {
   public:
    Exception(const std::string &description, Status status = Status::ERROR_UNKNOWN)
        : m_Description(description), m_What(getExceptionType() + ":" + getDescription()), m_Status(status) {
    }

    virtual std::string getExceptionType() const {
        return "Generic exception";
    }

    const std::string &getDescription() const {
        return m_Description;
    }

    virtual const char *what() const throw() {
        return m_What.c_str();
    }

    Status getStatus() const {
        return m_Status;
    }

   protected:
    std::string m_Description;
    std::string m_What;
    Status m_Status;
};
}  // namespace ccad
