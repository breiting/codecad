#pragma once
#include <string>

#include "runtime/Assembly.hpp"

namespace io {
// Schreibt STL/STEP für alle Parts und ein manifest.json in 'outdir'.
// Dateinamen: <assembly>__<part>.stl / .step
bool WriteAssemblyManifest(const runtime::Assembly& asmbl, const std::string& outdir, bool write_step = true);
}  // namespace io
