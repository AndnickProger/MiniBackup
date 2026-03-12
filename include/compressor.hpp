#pragma once

#include <boost/filesystem.hpp>
#include <string>

namespace backup
{
    namespace fs = boost::filesystem;

    struct Compressor
    {
        static constexpr int defaultCompressionLevel = 3;

        Compressor() noexcept = default;

        ~Compressor() noexcept = default;

        Compressor(const Compressor &) = delete;

        Compressor &operator=(const Compressor &) = delete;

        void compressFile(const fs::path &source, const fs::path &destination);
    };
}
