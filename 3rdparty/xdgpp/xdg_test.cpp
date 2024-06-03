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

#include "xdg.hpp"

#include "catch2/catch.hpp"

#include <filesystem>

template <typename T>
void TestSingleDirectory(T function, const char *env_name,
                         const std::filesystem::path &default_path) {
    unsetenv(env_name);
    xdg::BaseDirectories dirs;
    CHECK(std::bind(function, &dirs)() == default_path);

    const auto *new_env = "/tmp/mydir";
    setenv(env_name, new_env, 1);
    dirs = xdg::BaseDirectories{};
    CHECK(std::bind(function, &dirs)() == std::filesystem::path{new_env});

    setenv(env_name, &new_env[1], 1);
    dirs = xdg::BaseDirectories{};
    CHECK(std::bind(function, &dirs)() == default_path);
}

template <typename T>
void TestMultipleDirectories(
    T function, const char *env_name,
    const std::vector<std::filesystem::path> &default_paths) {
    unsetenv(env_name);
    xdg::BaseDirectories dirs;
    CHECK(std::bind(function, &dirs)() == default_paths);

    const auto *new_env = "/tmp/mydir:/tmp/mydir2";
    setenv(env_name, new_env, 1);
    dirs = xdg::BaseDirectories{};
    auto paths =
        std::vector<std::filesystem::path>{"/tmp/mydir", "/tmp/mydir2"};
    CHECK(std::bind(function, &dirs)() == paths);

    new_env = "tmp/mydir:tmp/mydir1";
    setenv(env_name, new_env, 1);
    dirs = xdg::BaseDirectories{};
    CHECK(std::bind(function, &dirs)() == default_paths);

    new_env = "/tmp/mydir:tmp/mydir1";
    setenv(env_name, new_env, 1);
    dirs = xdg::BaseDirectories{};
    CHECK(std::bind(function, &dirs)() ==
          std::vector<std::filesystem::path>{
              std::filesystem::path{"/tmp/mydir"}});
}

TEST_CASE("xdg::Dirs") {
    SECTION("XDG_DATA_HOME") {
        TestSingleDirectory(&xdg::BaseDirectories::DataHome, "XDG_DATA_HOME",
                            std::filesystem::path{getenv("HOME")} / ".local" /
                                "share");
    }
    SECTION("XDG_CONFIG_HOME") {
        TestSingleDirectory(&xdg::BaseDirectories::ConfigHome,
                            "XDG_CONFIG_HOME",
                            std::filesystem::path{getenv("HOME")} / ".config");
    }
    SECTION("XDG_DATA_DIRS") {
        TestMultipleDirectories(&xdg::BaseDirectories::Data, "XDG_DATA_DIRS",
                                {std::filesystem::path{"/usr/local/share"},
                                 std::filesystem::path{"/usr/share"}});
    }

    SECTION("XDG_CONFIG_DIRS") {
        TestMultipleDirectories(&xdg::BaseDirectories::Config,
                                "XDG_CONFIG_DIRS",
                                {std::filesystem::path{"/etc/xdg"}});
    }
    SECTION("XDG_CACHE_HOME") {
        TestSingleDirectory(&xdg::BaseDirectories::CacheHome, "XDG_CACHE_HOME",
                            std::filesystem::path{getenv("HOME")} / ".cache");
    }

    SECTION("XDG_RUNTIME_DIR") {
        std::filesystem::path new_dir{std::filesystem::current_path()};
        new_dir /= "runtime_dir";
        if (std::filesystem::exists(new_dir)) {
            // Clean up previous tests
            std::filesystem::remove(new_dir);
        }
        setenv("XDG_RUNTIME_DIR", new_dir.c_str(), 1);
        CHECK_THROWS_AS(xdg::BaseDirectories{}, xdg::BaseDirectoryException);

        // Create directory
        std::filesystem::create_directory(new_dir);
        // Apply 0777 mode
        std::filesystem::permissions(new_dir,
                                     std::filesystem::perms::owner_all |
                                         std::filesystem::perms::group_all

                                         | std::filesystem::perms::others_all,
                                     std::filesystem::perm_options::add);
        REQUIRE_THROWS_AS(xdg::BaseDirectories{}, xdg::BaseDirectoryException);

        // Remove group_all and other_all, so that permissions will be 0700
        std::filesystem::permissions(new_dir,
                                     std::filesystem::perms::group_all

                                         | std::filesystem::perms::others_all,
                                     std::filesystem::perm_options::remove);

        xdg::BaseDirectories *dirs = nullptr;
        REQUIRE_NOTHROW(dirs = new xdg::BaseDirectories{});
        REQUIRE(dirs->Runtime());
        CHECK(dirs->Runtime().value() == new_dir);

        std::filesystem::remove(new_dir);
        delete dirs;
    }
}
