#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace utils {

std::filesystem::path ExecutablePath();
std::vector<std::string> DefaultInstallLuaPathsFromExe();
std::string DefaultInstallFontsPath();
bool WriteTextFile(const std::filesystem::path& p, const std::string& content);

}  // namespace utils
