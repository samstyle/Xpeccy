#pragma once

// One-shot filesystem migration primitives used by conf_init and profile
// loading to move pre-XDG layouts into the new per-kind dirs. Kept in one
// place so the rename-or-copy-on-EXDEV idiom lives in exactly one function
// and so every legacy-window comment sits next to the helpers it uses. The
// whole header can be deleted when the XDG transition window closes.

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

namespace fs = std::filesystem;

namespace migrate {

// Whether a move targets a single regular file or a whole directory tree.
// Selects which copy/remove pair to use on the cross-device fallback path.
enum class MoveKind { SingleFile, DirectoryTree };

// Try fs::rename; if the target is on another filesystem (EXDEV), fall back
// to copy-then-remove with the kind-appropriate operations. Logs the attempt
// and any final error, then clears `ec` so callers don't carry the failure
// forward. Expects `to.parent_path()` to already exist.
void moveAcrossDevices(const fs::path &from, const fs::path &to, MoveKind kind,
                       std::string_view what, std::error_code &ec);

// If `from` exists and `to` does not, move `from` → `to`. No-op otherwise.
// Thin convenience around moveAcrossDevices for the "legacy file hanging
// around; move it if present, ignore it if not" idiom.
void migrateSingleFile(const fs::path &from, const fs::path &to,
                       std::string_view what, std::error_code &ec);

// Move an existing directory to a new location. No-op if `from` doesn't
// exist or `to` already does. Falls back to recursive copy + remove on
// cross-device moves.
void migrateDir(const fs::path &from, const fs::path &to,
                std::string_view what, std::error_code &ec);

// Move every regular file whose extension matches `ext` (case-insensitive,
// dot included, e.g. ".map") from `from` into `to`. Preserves filenames;
// skips files whose destination already exists. Intended for flattening
// pre-subdir resource files sitting at the root of confDir.
void migrateFilesByExtension(const fs::path &from, const fs::path &to,
                             std::string_view ext, std::string_view what,
                             std::error_code &ec);

} // namespace migrate
