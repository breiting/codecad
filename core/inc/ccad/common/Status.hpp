#pragma once

namespace ccad {

/**
 * Status which is used in the core library
 */
enum class Status {
    SUCCESS = 0,
    ERROR_OCCT = 1,

    ERROR_UNKNOWN = 0x7fffff01  // catch-all
};
}  // namespace ccad
