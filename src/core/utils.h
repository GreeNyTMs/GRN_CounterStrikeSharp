#pragma once

#include <public/eiface.h>
#include <string>
#include <filesystem>
#include <regex>
#include <algorithm>
#include <vector>

#include "core/globals.h"

namespace counterstrikesharp {
namespace utils {

static std::string gameDirectory;
inline std::string GameDirectory()
{
    if (gameDirectory.empty())
    {
        CBufferStringGrowable<255> gamePath;
        globals::engine->GetGameDir(gamePath);
        gameDirectory = std::string(gamePath.Get());
    }

    return gameDirectory;
}

// clang-format off
inline std::string NormalizeRelativePath(const std::string& path)
{
    std::string processedPath = path;

    processedPath.erase(processedPath.begin(), std::find_if(processedPath.begin(), processedPath.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));

    processedPath.erase(std::find_if(processedPath.rbegin(), processedPath.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), processedPath.end());

    processedPath = std::regex_replace(processedPath, std::regex(R"([\\/]+)"), "/");

    if (!processedPath.empty())
    {
        if (processedPath[0] != '/')
        {
            processedPath = "/" + processedPath;
        }
        if (processedPath.back() == '/' && processedPath.length() > 1)
        {
            processedPath.pop_back();
        }
    }

    return processedPath;
}

inline bool TrySetRelativeDirectory(const std::string& path, std::string& storedPath, bool& isInitialized)
{
    if (path.empty())
    {
        return false;
    }

    const std::string fullPath = GameDirectory() + path;
    if (std::filesystem::exists(fullPath) && std::filesystem::is_directory(fullPath))
    {
        storedPath = path;
        isInitialized = true;
        return true;
    }

    return false;
}

inline std::string RelativeDirectory(const std::string& initPath = "")
{
    static std::string storedPath;
    static bool isInitialized = false;

    if (!initPath.empty() && !isInitialized)
    {
        const std::string processedPath = NormalizeRelativePath(initPath);

        std::vector<std::string> candidatePaths;
        candidatePaths.push_back(processedPath);

        // After some CS2 updates IVEngineServer::GetGameDir() can resolve to the
        // server root "game" directory instead of the mod directory "game/csgo".
        // In that case the historical default "/addons/counterstrikesharp" must
        // be resolved as "/csgo/addons/counterstrikesharp".
        if (processedPath.rfind("/csgo/", 0) != 0)
        {
            candidatePaths.push_back("/csgo" + processedPath);
        }

        for (const std::string& candidatePath : candidatePaths)
        {
            if (TrySetRelativeDirectory(candidatePath, storedPath, isInitialized))
            {
                return storedPath;
            }
        }

        return "NotFound";
    }

    if (!isInitialized)
    {
        std::vector<std::string> candidatePaths;
        candidatePaths.push_back("/addons/counterstrikesharp");
        candidatePaths.push_back("/csgo/addons/counterstrikesharp");

        for (const std::string& candidatePath : candidatePaths)
        {
            if (TrySetRelativeDirectory(candidatePath, storedPath, isInitialized))
            {
                return storedPath;
            }
        }
    }

    return isInitialized ? storedPath : "/addons/counterstrikesharp";
}
// clang-format on

inline std::string GetRootDirectory() { return GameDirectory() + RelativeDirectory(); }
inline std::string PluginsDirectory() { return GameDirectory() + RelativeDirectory() + "/plugins"; }
inline std::string ConfigsDirectory() { return GameDirectory() + RelativeDirectory() + "/configs"; }
inline std::string GamedataDirectory() { return GameDirectory() + RelativeDirectory() + "/gamedata"; }

} // namespace utils
} // namespace counterstrikesharp
