#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace io {

std::filesystem::path ExecutablePath();
std::vector<std::string> DefaultInstallLuaPathsFromExe();
std::string DefaultInstallFontsPath();
bool WriteTextFile(const std::filesystem::path& p, const std::string& content, std::string* err = nullptr);
}  // namespace io
