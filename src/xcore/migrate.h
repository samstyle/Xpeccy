#pragma once

// One-shot filesystem migration primitives used by conf_init and profile
// loading to move pre-XDG layouts into the new per-kind dirs. Kept in one
// place so the rename-or-copy-on-EXDEV idiom lives in exactly one function
// and so every legacy-window comment sits next to the helpers it uses. The
// whole header can be deleted when the XDG transition window closes.

#include <filesystem>
#include <string_view>

namespace fs = std::filesystem;

// Public migration API — three "what to migrate" shapes: a single file, a
// whole directory, or every file with a given extension. All three share an
// internal rename-or-copy-on-EXDEV engine; callers treat these as
// fire-and-forget (error handling and logging are internal).
namespace migrate {

// If `from` exists and `to` does not, move `from` → `to`. No-op otherwise.
// The "legacy file hanging around; move it if present, ignore it if not"
// idiom.
void migrateSingleFile(const fs::path &from, const fs::path &to,
                       std::string_view what);

// Move an existing directory to a new location. No-op if `from` doesn't
// exist or `to` already does. Falls back to recursive copy + remove on
// cross-device moves.
void migrateDir(const fs::path &from, const fs::path &to,
                std::string_view what);

// Move every regular file whose extension matches `ext` (case-insensitive,
// dot included, e.g. ".map") from `from` into `to`. Preserves filenames;
// skips files whose destination already exists. Intended for flattening
// pre-subdir resource files sitting at the root of confDir.
void migrateFilesByExtension(const fs::path &from, const fs::path &to,
                             std::string_view ext, std::string_view what);

} // namespace migrate
