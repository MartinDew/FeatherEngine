module;

#include <filesystem>

export module feather.core:framework.path;

export namespace feather {
namespace FileSystem = std::filesystem;
using Path = std::filesystem::path;
}
