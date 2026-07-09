#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <public/eiface.h>
#include <regex>
#include <string>
#include <system_error>

#include "core/globals.h"

namespace counterstrikesharp {
namespace utils {

static std::string gameDirectory;

inline bool IsValidDirectory(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }

    std::error_code ec;
    return std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec);
}

inline std::string CanonicalOrOriginal(const std::string& path)
{
    std::error_code ec;
    auto canonicalPath = std::filesystem::weakly_canonical(path, ec);
    if (!ec && !canonicalPath.empty())
    {
        return canonicalPath.string();
    }

    return path;
}

inline std::string NormalizePath(std::string path)
{
    path.erase(path.begin(), std::find_if(path.begin(), path.end(), [](unsigned char ch) {
                   return !std::isspace(ch);
               }));

    path.erase(std::find_if(path.rbegin(), path.rend(), [](unsigned char ch) {
                   return !std::isspace(ch);
               }).base(),
               path.end());

    path = std::regex_replace(path, std::regex(R"([\\/]+)"), "/");

    if (!path.empty() && path.back() == '/' && path.length() > 1)
    {
        path.pop_back();
    }

    return path;
}

inline std::string GameDirectory()
{
    if (gameDirectory.empty())
    {
        CBufferStringGrowable<255> gamePath;
        globals::engine->GetGameDir(gamePath);
        gameDirectory = CanonicalOrOriginal(std::string(gamePath.Get()));
    }

    return gameDirectory;
}

inline std::string ModuleRootDirectory()
{
#ifndef _WIN32
    std::ifstream maps("/proc/self/maps");
    std::string line;

    while (std::getline(maps, line))
    {
        if (line.find("counterstrikesharp.so") == std::string::npos)
        {
            continue;
        }

        auto pathStart = line.find('/');
        if (pathStart == std::string::npos)
        {
            continue;
        }

        std::filesystem::path modulePath(line.substr(pathStart));
        auto rootPath = modulePath.parent_path().parent_path().parent_path();

        if (IsValidDirectory(rootPath.string()))
        {
            return CanonicalOrOriginal(rootPath.string());
        }
    }
#endif

    return {};
}

inline std::string ResolveConfiguredDirectory(const std::string& initPath)
{
    std::string processedPath = NormalizePath(initPath);

    if (processedPath.empty())
    {
        return {};
    }

    if (IsValidDirectory(processedPath))
    {
        return CanonicalOrOriginal(processedPath);
    }

    if (processedPath[0] != '/')
    {
        processedPath = "/" + processedPath;
    }

    std::string gameDir = GameDirectory();

    std::string candidate = gameDir + processedPath;
    if (IsValidDirectory(candidate))
    {
        return CanonicalOrOriginal(candidate);
    }

    candidate = gameDir + "/csgo" + processedPath;
    if (IsValidDirectory(candidate))
    {
        return CanonicalOrOriginal(candidate);
    }

    std::filesystem::path gamePath(gameDir);
    candidate = (gamePath.parent_path().string() + processedPath);
    if (IsValidDirectory(candidate))
    {
        return CanonicalOrOriginal(candidate);
    }

    return {};
}

inline std::string RelativeDirectory(const std::string& initPath = "")
{
    static std::string storedPath;
    static bool isInitialized = false;

    if (isInitialized)
    {
        return storedPath;
    }

    std::string moduleRoot = ModuleRootDirectory();
    if (IsValidDirectory(moduleRoot))
    {
        storedPath = moduleRoot;
        isInitialized = true;
        return storedPath;
    }

    if (!initPath.empty())
    {
        std::string configuredRoot = ResolveConfiguredDirectory(initPath);
        if (IsValidDirectory(configuredRoot))
        {
            storedPath = configuredRoot;
            isInitialized = true;
            return storedPath;
        }

        return "NotFound";
    }

    return "NotFound";
}

inline std::string GetRootDirectory() { return RelativeDirectory(); }
inline std::string PluginsDirectory() { return RelativeDirectory() + "/plugins"; }
inline std::string ConfigsDirectory() { return RelativeDirectory() + "/configs"; }
inline std::string GamedataDirectory() { return RelativeDirectory() + "/gamedata"; }

} // namespace utils
} // namespace counterstrikesharp