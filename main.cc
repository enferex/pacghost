#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
namespace fs = std::filesystem;

const static std::string DEFAULT_PACMAN_ROOT = "/var/lib/pacman";

struct PkgStats {
  PkgStats() : name(""), size(0), lastUsed(0) {}
  PkgStats(const std::string &n, size_t sz, size_t accessTime)
      : name(n), size(sz), lastUsed(accessTime) {}
  std::string name;
  size_t size;
  time_t lastUsed;  // Last access time in seconds (unix timestamp).
};

enum SortType {
  Size,        // Sort by the largest to smallest package.
  MostRecent,  // Sort by the most recently used package.
  LeastRecent  // Sort by the least recently used package.
};

static void usage(const char *execname) {
  std::cout << "Usage: " << execname << " [-h] [-n <num>] [-s | -t | -T]"
            << " [<pacman DBPath>] (default: " << DEFAULT_PACMAN_ROOT << ')'
            << std::endl
            << "  -h:       This help message." << std::endl
            << "  -n <num>: Number of results." << std::endl
            << "  -s:       Sort by largest package size." << std::endl
            << "  -t:       Sort by package most recently used." << std::endl
            << "  -T:       Sort by package least recently used." << std::endl;
}

static void scan(fs::path root, int nResults, SortType sort) {
  if (!fs::is_directory(root)) {
    std::cerr << "Invalid directory path: " << root << std::endl;
    exit(EXIT_FAILURE);
  }

  // For each package listed in pacman's DBPath.
  size_t nSkipped = 0;
  std::vector<PkgStats> packages;
  root /= "local";
  for (auto &item : fs::directory_iterator(root)) {
    size_t packageSize = 0;
    time_t mostRecentAccess = 0;

    // Read the 'files' file to obtain a list of files in this package.
    auto itemPath = item.path();
    itemPath /= "files";
    std::ifstream fp(itemPath);

    // For each file that makes up this package.
    for (std::string line; std::getline(fp, line);) {
      fs::path file("/" + line);
      try {
        if (fs::is_regular_file(file) && !fs::is_directory(file)) {
          packageSize += fs::file_size(file);

          struct stat st;
          if (stat(file.c_str(), &st) == -1) {
            ++nSkipped;
            continue;
          }
          mostRecentAccess = std::max(mostRecentAccess, st.st_atime);
        }
      } catch (fs::filesystem_error &e) {
        ++nSkipped;
        continue;
      }
    }
    packages.emplace_back(item.path().filename(), packageSize,
                          mostRecentAccess);
  }

  // Sort.
  switch (sort) {
    case SortType::Size:
      std::sort(packages.begin(), packages.end(),
                [](PkgStats &a, PkgStats &b) { return a.size > b.size; });
      break;
    case SortType::MostRecent:
      std::sort(packages.begin(), packages.end(), [](PkgStats &a, PkgStats &b) {
        return a.lastUsed > b.lastUsed;
      });
      break;
    case SortType::LeastRecent:
      std::sort(packages.begin(), packages.end(), [](PkgStats &a, PkgStats &b) {
        return a.lastUsed < b.lastUsed;
      });
      break;
  }

  // Display.
  std::cout << "#, Package, Bytes, Last Access Time" << std::endl;
  for (size_t i = 0; i < (size_t)nResults && i < packages.size(); ++i) {
    auto &pkg = packages[i];
    std::cout << i + 1 << ", " << pkg.name << ", " << pkg.size << " bytes, "
              << std::ctime(&pkg.lastUsed);
  }
}

int main(int argc, char **argv) {
  int opt = 0, nResults = 10;
  const char *root = DEFAULT_PACMAN_ROOT.c_str();
  SortType sort = SortType::Size;
  while ((opt = getopt(argc, argv, "hn:stT")) != -1) {
    switch (opt) {
      case 'h':
        usage(argv[0]);
        return 0;
      case 'n':
        nResults = std::atoi(optarg);
        break;
      case 's':
        sort = SortType::Size;
        break;
      case 't':
        sort = SortType::MostRecent;
        break;
      case 'T':
        sort = SortType::LeastRecent;
        break;
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  if (nResults < 0) {
    std::cerr << "Invalid -n option." << std::endl;
    return 1;
  }
  if (optind < argc) root = argv[optind];

  scan(root, nResults, sort);
  return 0;
}
