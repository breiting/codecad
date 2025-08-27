// app/src/Paths.hpp
#pragma once
#include <filesystem>
#include <string>
#include <vector>

std::filesystem::path ExecutablePath();
std::vector<std::string> DefaultInstallLuaPathsFromExe();
bool WriteTextFile(const std::filesystem::path& p, const std::string& content, std::string* err = nullptr);
