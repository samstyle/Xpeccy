/*
 * Copyright © 2020 Danilo Spinella <oss@danyspin97.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <unistd.h>

namespace xdg {

class BaseDirectoryException : public std::exception {
public:
    explicit BaseDirectoryException(std::string msg) : msg_(std::move(msg)) {}

    [[nodiscard]] auto what() const noexcept -> const char * override {
        return msg_.c_str();
    }
    [[nodiscard]] auto msg() const noexcept -> std::string { return msg_; }

private:
    const std::string msg_;
};

class BaseDirectories {
public:
    BaseDirectories() {
        const char *home_env = getenv("HOME");
        if (home_env == nullptr) {
            throw BaseDirectoryException("$HOME must be set");
        }
        home_ = std::filesystem::path{home_env};

        data_home_ = GetAbsolutePathFromEnvOrDefault(
            "XDG_DATA_HOME", home_ / ".local" / "share");
        config_home_ = GetAbsolutePathFromEnvOrDefault("XDG_CONFIG_HOME",
                                                       home_ / ".config");
        data_ = GetPathsFromEnvOrDefault("XDG_DATA_DIRS",
                                         std::vector<std::filesystem::path>{
                                             "/usr/local/share", "/usr/share"});
        config_ = GetPathsFromEnvOrDefault(
            "XDG_CONFIG_DIRS", std::vector<std::filesystem::path>{"/etc/xdg"});
        cache_home_ =
            GetAbsolutePathFromEnvOrDefault("XDG_CACHE_HOME", home_ / ".cache");

        SetRuntimeDir();
    }

    static auto GetInstance() -> BaseDirectories & {
        static BaseDirectories dirs;

        return dirs;
    }

    [[nodiscard]] auto DataHome() -> const std::filesystem::path & {
        return data_home_;
    }
    [[nodiscard]] auto ConfigHome() -> const std::filesystem::path & {
        return config_home_;
    }
    [[nodiscard]] auto Data() -> const std::vector<std::filesystem::path> & {
        return data_;
    }
    [[nodiscard]] auto Config() -> const std::vector<std::filesystem::path> & {
        return config_;
    }
    [[nodiscard]] auto CacheHome() -> const std::filesystem::path & {
        return cache_home_;
    }
    [[nodiscard]] auto Runtime()
        -> const std::optional<std::filesystem::path> & {
        return runtime_;
    }

private:
    void SetRuntimeDir() {
        const char *runtime_env = getenv("XDG_RUNTIME_DIR");
        if (runtime_env != nullptr) {
            std::filesystem::path runtime_dir{runtime_env};
            if (runtime_dir.is_absolute()) {
                if (!std::filesystem::exists(runtime_dir)) {
                    throw BaseDirectoryException(
                        "$XDG_RUNTIME_DIR must exist on the system");
                }

                auto runtime_dir_perms =
                    std::filesystem::status(runtime_dir).permissions();
                using perms = std::filesystem::perms;
                // Check XDG_RUNTIME_DIR permissions are 0700
                if (((runtime_dir_perms & perms::owner_all) == perms::none) ||
                    ((runtime_dir_perms & perms::group_all) != perms::none) ||
                    ((runtime_dir_perms & perms::others_all) != perms::none)) {
                    throw BaseDirectoryException(
                        "$XDG_RUNTIME_DIR must have 0700 as permissions");
                }
                runtime_.emplace(runtime_dir);
            }
        }
    }

    static auto
    GetAbsolutePathFromEnvOrDefault(const char *env_name,
                                    std::filesystem::path &&default_path)
        -> std::filesystem::path {
        const char *env_var = getenv(env_name);
        if (env_var == nullptr) {
            return std::move(default_path);
        }
        auto path = std::filesystem::path{env_var};
        if (!path.is_absolute()) {
            return std::move(default_path);
        }

        return path;
    }

    static auto
    GetPathsFromEnvOrDefault(const char *env_name,
                             std::vector<std::filesystem::path> &&default_paths)
        -> std::vector<std::filesystem::path> {
        auto *env = getenv(env_name);
        if (env == nullptr) {
            return std::move(default_paths);
        }
        std::string paths{env};

        std::vector<std::filesystem::path> dirs{};
        size_t start = 0;
        size_t pos = 0;
        while ((pos = paths.find_first_of(':', start)) != std::string::npos) {
            std::filesystem::path current_path{
                paths.substr(start, pos - start)};
            if (current_path.is_absolute() &&
                !VectorContainsPath(dirs, current_path)) {
                dirs.emplace_back(current_path);
            }
            start = pos + 1;
        }
        std::filesystem::path current_path{paths.substr(start, pos - start)};
        if (current_path.is_absolute() &&
            !VectorContainsPath(dirs, current_path)) {
            dirs.emplace_back(current_path);
        }

        if (dirs.empty()) {
            return std::move(default_paths);
        }

        return dirs;
    }

    static auto
    VectorContainsPath(const std::vector<std::filesystem::path> &paths,
                       const std::filesystem::path &path) -> bool {
        return std::find(std::begin(paths), std::end(paths), path) !=
               paths.end();
    }

    std::filesystem::path home_;

    std::filesystem::path data_home_;
    std::filesystem::path config_home_;
    std::vector<std::filesystem::path> data_;
    std::vector<std::filesystem::path> config_;
    std::filesystem::path cache_home_;
    std::optional<std::filesystem::path> runtime_;

}; // namespace xdg

[[nodiscard]] inline auto DataHomeDir() -> const std::filesystem::path & {
    return BaseDirectories::GetInstance().DataHome();
}
[[nodiscard]] inline auto ConfigHomeDir() -> const std::filesystem::path & {
    return BaseDirectories::GetInstance().ConfigHome();
}
[[nodiscard]] inline auto DataDirs()
    -> const std::vector<std::filesystem::path> & {
    return BaseDirectories::GetInstance().Data();
}
[[nodiscard]] inline auto ConfigDirs()
    -> const std::vector<std::filesystem::path> & {
    return BaseDirectories::GetInstance().Config();
}
[[nodiscard]] inline auto CacheHomeDir() -> const std::filesystem::path & {
    return BaseDirectories::GetInstance().CacheHome();
}
[[nodiscard]] inline auto RuntimeDir()
    -> const std::optional<std::filesystem::path> & {
    return BaseDirectories::GetInstance().Runtime();
}

} // namespace xdg
