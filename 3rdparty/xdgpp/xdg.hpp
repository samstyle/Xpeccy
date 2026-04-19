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
#include <fstream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
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
        // Local extension: StateHomeDir is defined in v0.8 of the XDG Base
        // Directory Specification. Default is $HOME/.local/state.
        state_home_ = GetAbsolutePathFromEnvOrDefault(
            "XDG_STATE_HOME", home_ / ".local" / "state");

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
    [[nodiscard]] auto StateHome() -> const std::filesystem::path & {
        return state_home_;
    }
    [[nodiscard]] auto Runtime()
        -> const std::optional<std::filesystem::path> & {
        return runtime_;
    }
    [[nodiscard]] auto Home() -> const std::filesystem::path & {
        return home_;
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
    std::filesystem::path state_home_;
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
// Local extension: per XDG Base Directory Specification v0.8.
[[nodiscard]] inline auto StateHomeDir() -> const std::filesystem::path & {
    return BaseDirectories::GetInstance().StateHome();
}
[[nodiscard]] inline auto RuntimeDir()
    -> const std::optional<std::filesystem::path> & {
    return BaseDirectories::GetInstance().Runtime();
}

namespace detail {

// Pure: strip leading/trailing ASCII whitespace. View-in, view-out.
inline auto trim(std::string_view s) -> std::string_view {
    const auto space = [](char c) {
        return std::isspace(static_cast<unsigned char>(c));
    };
    while (!s.empty() && space(s.front())) s.remove_prefix(1);
    while (!s.empty() && space(s.back()))  s.remove_suffix(1);
    return s;
}

// Pure: strip a matched pair of surrounding double quotes. No-op otherwise.
inline auto unquote(std::string_view s) -> std::string_view {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s.remove_prefix(1);
        s.remove_suffix(1);
    }
    return s;
}

// Pure: expand a leading "$HOME" or "$HOME/" — the only substitution that
// xdg-user-dirs-update emits into user-dirs.dirs.
inline auto expandHome(std::string_view v, const std::string &home)
    -> std::string {
    if (v == "$HOME") return home;
    if (v.substr(0, 6) == "$HOME/") return home + std::string{v.substr(5)};
    return std::string{v};
}

// Pure: one line of user-dirs.dirs + the key to match + $HOME → resolved path,
// or nullopt if the line is a comment/blank, doesn't assign `key`, or has an
// empty value.
inline auto parseUserDirLine(std::string_view line,
                             std::string_view key,
                             const std::string &home)
    -> std::optional<std::filesystem::path> {
    line = trim(line);
    if (line.empty() || line.front() == '#') return std::nullopt;
    const auto eq = line.find('=');
    if (eq == std::string_view::npos) return std::nullopt;
    if (trim(line.substr(0, eq)) != key) return std::nullopt;
    const auto value = unquote(trim(line.substr(eq + 1)));
    if (value.empty()) return std::nullopt;
    return std::filesystem::path{expandHome(value, home)};
}

} // namespace detail

// Local extension: parse $XDG_CONFIG_HOME/user-dirs.dirs (xdg-user-dirs spec)
// and return the value for `key` (e.g. "XDG_PICTURES_DIR"). Expands a leading
// $HOME; strips surrounding double quotes; skips comments and malformed lines.
// Returns nullopt if the file doesn't exist, the key isn't present, or the
// value would be empty. Callers typically layer a per-key default on top.
[[nodiscard]] inline auto UserDir(std::string_view key)
    -> std::optional<std::filesystem::path> {
    const auto cfg = BaseDirectories::GetInstance().ConfigHome() / "user-dirs.dirs";
    std::ifstream file(cfg);
    if (!file) return std::nullopt;
    const std::string &home = BaseDirectories::GetInstance().Home().string();
    std::string line;
    while (std::getline(file, line)) {
        if (auto hit = detail::parseUserDirLine(line, key, home)) return hit;
    }
    return std::nullopt;
}

// Local extension: Pictures directory per xdg-user-dirs. Falls back to
// $HOME/Pictures if the user-dirs config is missing or unset.
[[nodiscard]] inline auto PicturesDir() -> std::filesystem::path {
    if (auto p = UserDir("XDG_PICTURES_DIR")) return *p;
    return BaseDirectories::GetInstance().Home() / "Pictures";
}

} // namespace xdg
