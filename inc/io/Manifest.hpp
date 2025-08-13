#pragma once
#include <string>

#include "runtime/Assembly.hpp"

namespace io {
// Writes a JSON file storing different parts with potential explosion vectors for a later animation.
bool WriteAssemblyManifest(const runtime::Assembly& assembly, const std::string& outdir, bool writeStep = true);
}  // namespace io
