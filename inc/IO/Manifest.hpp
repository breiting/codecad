#pragma once
#include <string>

#include "Runtime/Assembly.hpp"

namespace IO {
// Schreibt STL/STEP für alle Parts und ein manifest.json in 'outdir'.
// Dateinamen: <assembly>__<part>.stl / .step
bool WriteAssemblyManifest(const Runtime::Assembly& asmbl, const std::string& outdir, bool write_step = true);
}  // namespace IO
