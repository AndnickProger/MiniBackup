#pragma once

#include <memory>
#include <filesystem>
#include <string>
#include <thread>
#include <boost/asio/thread_pool.hpp>

#include "storage.hpp"
#include "compressor.hpp"

namespace backup
{
    namespace fs = boost::filesystem;

    class BackupEngine
    {
    public:
        explicit BackupEngine(const std::string &dbPath,
                              size_t threadCount = std::thread::hardware_concurrency());

        ~BackupEngine() noexcept = default;

        BackupEngine(const BackupEngine &) = delete;

        BackupEngine &operator=(const BackupEngine &) = delete;

        void run(const fs::path &source, const fs::path &target);

        void wait(); 

    private:
        void processFile(const fs::path &filePath, const fs::path &targetDir);

        std::string calculateHash(const fs::path &path);

        std::unique_ptr<StorageManager> storage;
        Compressor compressor;

        boost::asio::thread_pool pool;
    };
}
