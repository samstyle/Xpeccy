# xdgpp

[![builds.sr.ht status](https://builds.sr.ht/~danyspin97/xdgpp.svg)](https://builds.sr.ht/~danyspin97/xdgpp?)
![Liberapay receiving](https://img.shields.io/liberapay/receives/danyspin97?logo=liberapay)

C++17 header-only implementation of the [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html).

## Installation

Copy `xdg.hpp` into your source tree.

## Usage

```cpp
#include <iostream>

#include "xdg.hpp"

int main() {
    std::cout << "$XDG_DATA_HOME = " << xdg::DataHomeDir() << std::endl;
    std::cout << "$XDG_CONFIG_HOME = " << xdg::ConfigHomeDir() << std::endl;
    std::cout << "$XDG_CACHE_HOME = " << xdg::CacheHomeDir() << std::endl;

    auto data_dirs = xdg::DataDirs();
    std::cout << "$XDG_DATA_DIRS = \"" << data_dirs.front().c_str();
    for (int i = 1; i < data_dirs.size(); i++) {
        std::cout << ":" << data_dirs.at(i).c_str();
    }
    std::cout << "\"" << std::endl;

    auto config_dirs = xdg::ConfigDirs();
    std::cout << "$XDG_CONFIG_DIRS = \"" << config_dirs.front().c_str();
    for (int i = 1; i < config_dirs.size(); i++) {
        std::cout << ":" << config_dirs.at(i).c_str();
    }
    std::cout << "\"" << std::endl;

    // XDG_RUNTIME_DIR might not be set, the API returns a std::optional
    auto runtime_dir = xdg::RuntimeDir();
    if (runtime_dir) {
        std::cout << "$XDG_RUNTIME_DIR = " << runtime_dir.value();
    }

    return 0;
}
```

Alternatively you can create and use an instance of `xdg::BaseDirectories` like:

```cpp
#include <iostream>

#include "xdg.hpp"

int main() {
    xdg::BaseDirectories dirs;
    std::cout << "$XDG_DATA_HOME = " << dirs.DataHome() << std::endl;
}
```

## Contributing

Pull requests are welcome.

**xdgpp** follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html); run `clang-format` on changes to automatically format the code.

## License

**xdgpp** is licensed under [the Mit License](https://mit-license.org/).

