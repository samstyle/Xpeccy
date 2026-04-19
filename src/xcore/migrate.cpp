#include "migrate.h"

#include <algorithm>
#include <cctype>
#include <iostream>

namespace migrate {

namespace {

std::string toLowerCopy(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(),
	               [](unsigned char c) { return std::tolower(c); });
	return s;
}

} // namespace

void moveAcrossDevices(const fs::path &from, const fs::path &to, MoveKind kind,
                       std::string_view what, std::error_code &ec) {
	std::cout << "Moving " << from << " -> " << to << std::endl;
	fs::rename(from, to, ec);
	if (ec == std::errc::cross_device_link) {
		ec.clear();
		if (kind == MoveKind::SingleFile) {
			fs::copy_file(from, to, ec);
		} else {
			fs::copy(from, to, fs::copy_options::recursive, ec);
		}
		if (!ec) {
			if (kind == MoveKind::SingleFile) fs::remove(from, ec);
			else                              fs::remove_all(from, ec);
		}
	}
	if (ec) {
		std::cout << "legacy " << what << " migration failed: "
		          << ec.message() << std::endl;
		ec.clear();
	}
}

void migrateSingleFile(const fs::path &from, const fs::path &to,
                       std::string_view what, std::error_code &ec) {
	if (!fs::exists(from, ec) || fs::exists(to, ec)) {
		ec.clear();
		return;
	}
	ec.clear();
	fs::create_directories(to.parent_path(), ec);
	ec.clear();
	moveAcrossDevices(from, to, MoveKind::SingleFile, what, ec);
}

void migrateDir(const fs::path &from, const fs::path &to,
                std::string_view what, std::error_code &ec) {
	if (!fs::exists(from, ec) || fs::exists(to, ec)) {
		ec.clear();
		return;
	}
	ec.clear();
	fs::create_directories(to.parent_path(), ec);
	ec.clear();
	moveAcrossDevices(from, to, MoveKind::DirectoryTree, what, ec);
}

void migrateFilesByExtension(const fs::path &from, const fs::path &to,
                             std::string_view ext, std::string_view what,
                             std::error_code &ec) {
	if (from == to) return;
	if (!fs::exists(from, ec) || !fs::is_directory(from, ec)) {
		ec.clear();
		return;
	}
	ec.clear();
	fs::create_directories(to, ec);
	ec.clear();
	const std::string wantExt = toLowerCopy(std::string{ext});
	for (auto it = fs::directory_iterator(from, ec);
	     !ec && it != fs::directory_iterator();
	     it.increment(ec)) {
		std::error_code fec;
		if (!it->is_regular_file(fec)) continue;
		if (toLowerCopy(it->path().extension().string()) != wantExt) continue;
		const fs::path dst = to / it->path().filename();
		if (fs::exists(dst, fec)) continue;        // don't clobber existing
		moveAcrossDevices(it->path(), dst, MoveKind::SingleFile, what, fec);
	}
	ec.clear();
}

} // namespace migrate
